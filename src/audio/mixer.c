/* Copyright 2024-2025 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <gba/audio.h>

#include <gba/interrupt.h>
#include <gba/timer.h>
#include <gba/dma.h>
#include <memory.h>
#include <math.h>

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)
#define MASTER_SOUND_CONTROL *((vu16 *) 0x04000084)

#define CHANNEL_COUNT 8
#define OUTPUT_COUNT  2
#define BUFFER_SIZE   768 // enough to last ~47ms at 16384 Hz

#define VOLUME_LEVELS 32
static_assert(
    -128 * VOLUME_LEVELS * CHANNEL_COUNT >= I16_MIN &&
    127 * VOLUME_LEVELS * CHANNEL_COUNT <= I16_MAX,
    "driver configuration allows overflow in temporary buffers"
);

static struct Channel {
    const i8 *data; // NULL if channel is inactive
    const i8 *end;

    u16 position;  // fixed-point, 1 = 0x1000
    u16 increment; // fixed-point, 1 = 0x1000

    u32 loop_length;
    u8 volume;
    i8 panning;

    bool paused;

    // cached volume per output, obtained from 'volume' and 'panning'
    u8 output_volume[OUTPUT_COUNT] ALIGNED(2);
} channels[CHANNEL_COUNT];

SBSS_SECTION
static i8 buffers[OUTPUT_COUNT][BUFFER_SIZE];

// Note: the effects of play, stop, pause, resume, volume and panning
// are delayed until the buffer is next updated.

THUMB
static i32 mixer_play(i32 channel, const void *sound, u32 length) {
    channels[channel].data = (const i8 *) sound;
    channels[channel].end  = (const i8 *) sound + length;

    channels[channel].position = 0;
    channels[channel].paused = false;

    return channel;
}

THUMB
static void mixer_stop(i32 channel) {
    channels[channel].data = NULL;
}

THUMB
static void mixer_pause(i32 channel) {
    channels[channel].paused = true;
}

THUMB
static void mixer_resume(i32 channel) {
    channels[channel].paused = false;
}

THUMB
static void mixer_loop(i32 channel, u32 loop_length) {
    channels[channel].loop_length = loop_length;
}

THUMB
static void mixer_pitch(i32 channel, u32 pitch) {
    if(pitch > AUDIO_PITCH_MAX)
        pitch = AUDIO_PITCH_MAX;

    channels[channel].increment = pitch;
}

static inline void update_output_volume(u32 channel) {
    const u32 volume  = channels[channel].volume;
    const i32 panning = channels[channel].panning;

    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        // calculate weight by either subtracting or adding panning
        u32 weight = AUDIO_VOLUME_MAX + (o == 0 ? +panning : -panning);

        if(weight > AUDIO_VOLUME_MAX)
            weight = AUDIO_VOLUME_MAX;

        u32 output_volume = weight * volume / AUDIO_VOLUME_MAX;
        output_volume /= (AUDIO_VOLUME_MAX / VOLUME_LEVELS);
        channels[channel].output_volume[o] = output_volume;
    }
}

THUMB
static void mixer_volume(i32 channel, u32 volume) {
    if(volume > AUDIO_VOLUME_MAX)
        volume = AUDIO_VOLUME_MAX;

    channels[channel].volume = volume;
    update_output_volume(channel);
}

THUMB
static void mixer_panning(i32 channel, i32 panning) {
    panning = math_clip(panning, AUDIO_PANNING_MIN, AUDIO_PANNING_MAX);

    channels[channel].panning = panning;
    update_output_volume(channel);
}

IWRAM_SECTION
static void timer1_isr(void) {
    // configure DMA transfers for each output
    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        const struct {
            void *fifo;
            u32 dma;
        } outputs[2] = {
            { .fifo = (void *) 0x040000a0, .dma = DMA1 },
            { .fifo = (void *) 0x040000a4, .dma = DMA2 }
        };

        dma_config(outputs[o].dma, &(struct DMA) {
            .dest_control = DMA_ADDR_FIXED,
            .src_control  = DMA_ADDR_INCREMENT,
            .chunk        = DMA_CHUNK_32_BIT,
            .start_timing = DMA_START_SPECIAL,
            .repeat       = true
        });
        dma_transfer(outputs[o].dma, outputs[o].fifo, buffers[o], 0);
    }
}

THUMB
static void mixer_init(void) {
    MASTER_SOUND_CONTROL = BIT(7);
    DIRECT_SOUND_CONTROL = BIT(2)  | BIT(3)  | // set volume to 100%
                           BIT(8)  | BIT(13) | // enable right and left
                           BIT(11) | BIT(15);  // reset FIFOs

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, TIMER_PRESCALER_1);
    timer_config(TIMER1, TIMER_CASCADE);

    // enable Timer 1 interrupt
    interrupt_toggle(IRQ_TIMER1, true);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // setup channels with default configuration
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        mixer_loop   (c, 0);
        mixer_pitch  (c, AUDIO_PITCH_NORMAL);
        mixer_volume (c, AUDIO_VOLUME_MAX);
        mixer_panning(c, 0);
    }

    // start Timer 1
    timer_start(TIMER1, BUFFER_SIZE);

    // Set default sample rate. This also starts Timer 0
    audio_sample_rate(0);
}

extern void _mixer_update(struct Channel *channels, void *buffers,
                          u32 length);

THUMB
static void mixer_update(void) {
    // These are indexes of the buffer, rounded down to a multiple of 4.
    // - start = first sample that needs updating
    // - stop  = sample about to be played, that should NOT be updated
    static u32 start = 0;
    const u32 stop = (BUFFER_SIZE - timer_get_counter(TIMER1)) & ~3;

    if(start < stop) {
        // update start...stop
        _mixer_update(channels, (u8 *) buffers + start, stop - start);
    } else if(start > stop) {
        // update start...BUFFER_SIZE
        _mixer_update(channels, (u8 *) buffers + start, BUFFER_SIZE - start);

        // update 0...stop
        _mixer_update(channels, buffers, stop);
    }

    // move 'start' forward
    start = stop;
}

static i32 mixer_available_channel(void) {
    for(u32 c = 0; c < CHANNEL_COUNT; c++)
        if(!channels[c].data)
            return c;
    return -1;
}

const struct _AudioDriver _audio_driver_mixer = {
    .init   = mixer_init,
    .update = mixer_update,

    .play = mixer_play,
    .stop = mixer_stop,

    .pause  = mixer_pause,
    .resume = mixer_resume,

    .loop    = mixer_loop,
    .pitch   = mixer_pitch,
    .volume  = mixer_volume,
    .panning = mixer_panning,

    .channel_count     = CHANNEL_COUNT,
    .available_channel = mixer_available_channel
};
