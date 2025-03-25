/* Copyright 2024 Vulcalien
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

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)
#define MASTER_SOUND_CONTROL *((vu16 *) 0x04000084)

#define OUTPUT_COUNT 2
#define BUFFER_SIZE  768 // enough to last ~47ms at 16384 Hz

static struct Channel {
    const i8 *data; // NULL if channel is inactive
    const i8 *end;

    u32 loop_length;
    u8 volume;
    i8 panning;

    // cached volume per output, obtained from 'volume' and 'panning'
    u8 output_volume[OUTPUT_COUNT];
} channels[AUDIO_CHANNEL_COUNT];

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

void audio_init(void) {
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
    audio_loop   (-1, 0);
    audio_volume (-1, AUDIO_VOLUME_MAX);
    audio_panning(-1, 0);

    // start Timer 1
    timer_start(TIMER1, BUFFER_SIZE);

    // Set default sample rate. This also starts Timer 0
    audio_sample_rate(0);
}

void audio_update(void) {
    if(!need_new_page)
        return;

    need_new_page = false;

    // TODO
}

// Note: the effects of play, stop, volume and panning are delayed until
// the buffer is next updated.

i32 audio_play(i32 channel, const void *sound, u32 length) {
    if(channel < 0) {
        // look for an available channel
        for(u32 c = 0; c < AUDIO_CHANNEL_COUNT; c++) {
            if(!channels[c].data) {
                channel = c;
                break;
            }
        }

        // if no channel is available, return
        if(channel < 0)
            return -1;
    } else if(channel >= AUDIO_CHANNEL_COUNT) {
        return -1;
    }

    struct Channel *channel_struct = &channels[channel];
    channel_struct->data = (const void *) sound;
    channel_struct->end  = (const void *) sound + length;

    return channel;
}

void _audio_stop(i32 channel) {
    channels[channel].data = NULL;
}

void _audio_loop(i32 channel, u32 loop_length) {
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

void _audio_volume(i32 channel, u32 volume) {
    if(volume > AUDIO_VOLUME_MAX)
        volume = AUDIO_VOLUME_MAX;

    channels[channel].volume = volume;
    update_output_volume(channel);
}

void _audio_panning(i32 channel, i32 panning) {
    if(panning < AUDIO_PANNING_MIN)
        panning = AUDIO_PANNING_MIN;
    if(panning > AUDIO_PANNING_MAX)
        panning = AUDIO_PANNING_MAX;

    channels[channel].panning = panning;
    update_output_volume(channel);
}
