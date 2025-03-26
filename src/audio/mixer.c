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

static struct Channel {
    const i8 *data; // NULL if channel is inactive
    const i8 *end;

    u32 loop_length;
    u8 volume;
    i8 panning;

    // cached volume per output, obtained from 'volume' and 'panning'
    u8 output_volume[OUTPUT_COUNT];
} channels[CHANNEL_COUNT];

static u32 buffer_page;
static bool need_new_page;

SBSS_SECTION
static i16 temp_buffers[OUTPUT_COUNT][BUFFER_SIZE];

SBSS_SECTION
static i8 buffers[2][OUTPUT_COUNT][BUFFER_SIZE];

IWRAM_SECTION
static void timer1_isr(void) {
    // reset FIFOs
    DIRECT_SOUND_CONTROL |= BIT(11) | BIT(15);

    // configure DMA transfers for each output
    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        u32 dma = (o == 0 ? DMA1 : DMA2);
        vi8 *fifo = (vi8 *) (o == 0 ? 0x040000a0 : 0x040000a4);

        dma_config(dma, &(struct DMA) {
            .start_timing = DMA_START_SPECIAL,
            .repeat = true
        });
        dma_transfer(dma, fifo, buffers[buffer_page][o], 0);
    }

    buffer_page ^= 1;
    need_new_page = true;
}

// Note: the effects of play, stop, volume and panning are delayed until
// the buffer is next updated.

THUMB
static i32 mixer_play(i32 channel, const void *sound, u32 length) {
    struct Channel *channel_struct = &channels[channel];
    channel_struct->data = (const i8 *) sound;
    channel_struct->end  = (const i8 *) sound + length;

    return channel;
}

THUMB
static void mixer_stop(i32 channel) {
    channels[channel].data = NULL;
}

THUMB
static void mixer_loop(i32 channel, u32 loop_length) {
    channels[channel].loop_length = loop_length;
}

static inline void update_output_volume(u32 channel) {
    struct Channel *channel_struct = &channels[channel];

    const u32 volume  = channel_struct->volume;
    const i32 panning = channel_struct->panning;

    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        // calculate weight by either subtracting or adding panning
        u32 weight = AUDIO_VOLUME_MAX + (o == 0 ? -panning : +panning);

        if(weight > AUDIO_VOLUME_MAX)
            weight = AUDIO_VOLUME_MAX;

        const u32 output_volume = weight * volume / AUDIO_VOLUME_MAX;
        channel_struct->output_volume[o] = output_volume;
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

static void mixer_init(void) {
    MASTER_SOUND_CONTROL = BIT(7);

    #if OUTPUT_COUNT == 2
    // stereo: enable one direction per Direct Sound output
    DIRECT_SOUND_CONTROL = BIT(2) | BIT(3)  | // set volume to 100%
                           BIT(8) | BIT(13);  // enable right and left
    #else
    // mono: only enable Direct Sound A
    DIRECT_SOUND_CONTROL = BIT(2) |         // set volume to 100%
                           BIT(8) | BIT(9); // enable right and left
    #endif

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, TIMER_PRESCALER_1);
    timer_config(TIMER1, TIMER_CASCADE);

    // enable Timer 1 interrupt
    interrupt_toggle(IRQ_TIMER1, true);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // setup channels with default configuration
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        mixer_loop   (c, 0);
        mixer_volume (c, AUDIO_VOLUME_MAX);
        mixer_panning(c, 0);
    }

    // start Timer 1
    timer_start(TIMER1, BUFFER_SIZE);

    // Set default sample rate. This also starts Timer 0
    audio_sample_rate(0);
}

IWRAM_SECTION
static void mixer_update(void) {
    if(!need_new_page)
        return;

    need_new_page = false;

    // clear temporary buffers
    memory_set_32(temp_buffers, 0, sizeof(temp_buffers));

    // add samples from active channels to temporary buffers
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        struct Channel *channel = &channels[c];
        if(!channel->data)
            continue;

        for(u32 s = 0; s < BUFFER_SIZE; s++) {
            // read sample and move data pointer forward
            const i8 sample = *(channel->data++);

            for(u32 o = 0; o < OUTPUT_COUNT; o++) {
                const u32 volume = channel->output_volume[o];
                temp_buffers[o][s] += sample * volume / AUDIO_VOLUME_MAX;
            }

            // if end of sound is reached, loop or stop
            if(channel->data >= channel->end) {
                if(channel->loop_length != 0) {
                    channel->data = channel->end - channel->loop_length;
                } else {
                    channel->data = NULL;
                    break;
                }
            }
        }
    }

    // write clipped data from temporary buffers to output buffers
    for(u32 s = 0; s < BUFFER_SIZE; s++) {
        for(u32 o = 0; o < OUTPUT_COUNT; o++) {
            buffers[buffer_page][o][s] = math_clip(
                temp_buffers[o][s], I8_MIN, I8_MAX
            );
        }
    }
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

    .loop    = mixer_loop,
    .volume  = mixer_volume,
    .panning = mixer_panning,

    .channel_count     = CHANNEL_COUNT,
    .available_channel = mixer_available_channel
};
