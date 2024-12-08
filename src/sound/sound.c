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
#include <gba/sound.h>

#include <gba/interrupt.h>
#include <gba/timer.h>
#include <gba/dma.h>
#include <memory.h>

//#define SOUND_STEREO

#ifdef SOUND_STEREO
    #define OUTPUT_COUNT 2
#else
    #define OUTPUT_COUNT 1
#endif

#define CHANNEL_COUNT 8
#define BUFFER_SIZE 256

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)
#define MASTER_SOUND_CONTROL *((vu16 *) 0x04000084)

static struct Channel {
    const i8 *data;
    const i8 *end;

    u16 position;  // 4.12 fixed-point (1 = 0x1000)
    u16 increment; // 4.12 fixed-point (1 = 0x1000)

    u8 output_volume[OUTPUT_COUNT];
    u8 volume;
    i8 panning;
} channels[CHANNEL_COUNT];

static u8 buffer_page = 0;

SBSS_SECTION
static i8 buffers[2][OUTPUT_COUNT][BUFFER_SIZE];

// DEBUG
#include <debug/mgba.h>
#include <debug/profiler.h>

IWRAM_SECTION
static void timer1_isr(void) {
    // reset FIFOs
    DIRECT_SOUND_CONTROL |= BIT(11) | BIT(15);

    // configure DMA transfers for each output
    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        u32 dma = (o == 0 ? DMA1 : DMA2);
        vu32 *fifo = (vu32 *) (o == 0 ? 0x040000a0 : 0x040000a4);

        dma_config(dma, &(struct DMA) {
            .start_timing = DMA_START_SPECIAL,
            .repeat = true
        });
        dma_transfer(dma, fifo, buffers[buffer_page][o], 0);
    }

    buffer_page ^= 1;

    // DEBUG
    mgba_open();
    profiler_start();
    sound_update();
    u32 t = profiler_stop();
    mgba_printf("%u", t);
}

void sound_init(void) {
    MASTER_SOUND_CONTROL = BIT(7); // enable sound

    #ifdef SOUND_STEREO
    // stereo: enable one direction per DMA channel
    DIRECT_SOUND_CONTROL = BIT(2) | BIT(3)  | // set volume to 100%
                           BIT(8) | BIT(13);  // enable right and left
    #else
    // mono: only enable DMA channel A
    DIRECT_SOUND_CONTROL = BIT(2) | // set volume to 100%
                           BIT(8) | // enable right
                           BIT(9);  // enable left
    #endif

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, TIMER_PRESCALER_1);
    timer_config(TIMER1, TIMER_CASCADE);

    // enable Timer 1 interrupt
    interrupt_toggle(IRQ_TIMER1, true);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // setup channels with default configuration
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        sound_volume(c, SOUND_VOLUME_MAX);
        sound_panning(c, 0);

        channels[c].increment = 4000;
    }

    // start Timer 1
    timer_start(TIMER1, BUFFER_SIZE);

    // Set default sample rate. This also starts Timer 0
    sound_sample_rate(0);
}

SBSS_SECTION
static i16 temp_buffers[OUTPUT_COUNT][BUFFER_SIZE];

IWRAM_SECTION
void sound_update(void) {
    // clear intermediate buffers
    memset32(temp_buffers, 0, OUTPUT_COUNT * BUFFER_SIZE * sizeof(i16));

    // fill intermediate buffers
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        struct Channel *channel = &channels[c];

        if(!channel->data)
            continue;

        // retrieve next samples
        for(u32 s = 0; s < BUFFER_SIZE; s++) {
            if(channel->data >= channel->end) {
                channel->data = NULL;
                break;
            }

            i32 raw_sample = *channel->data;

            channel->position += channel->increment;
            channel->data += channel->position / 0x1000;
            channel->position %= 0x1000;

            for(u32 o = 0; o < OUTPUT_COUNT; o++) {
                u32 output_volume = channel->output_volume[o];
                i32 sample = raw_sample * output_volume / SOUND_VOLUME_MAX;
                temp_buffers[o][s] += sample;
            }
        }
    }

    // apply clipping
    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        for(u32 s = 0; s < BUFFER_SIZE; s++) {
            i32 sample = temp_buffers[o][s];

            if(sample > +127) sample = +127;
            if(sample < -128) sample = -128;

            buffers[buffer_page][o][s] = sample;
        }
    }
}

void sound_play(const i8 *sound, u32 length, u32 channel) {
    if(channel >= CHANNEL_COUNT)
        return;

    struct Channel *channel_struct = &channels[channel];

    channel_struct->data = sound;
    channel_struct->end = sound + length;
}

static void update_output_volume(u32 channel) {
    struct Channel *channel_struct = &channels[channel];

    const i32 panning = channel_struct->panning;
    const u32 volume = channel_struct->volume;

    for(u32 o = 0; o < OUTPUT_COUNT; o++) {
        u32 weight = SOUND_VOLUME_MAX + (o == 0 ? -1 : +1) * panning;

        if(weight > SOUND_VOLUME_MAX)
            weight = SOUND_VOLUME_MAX;

        channel_struct->output_volume[o] = weight * volume / SOUND_VOLUME_MAX;
    }
}

void sound_volume(u32 channel, u32 volume) {
    if(channel >= CHANNEL_COUNT)
        return;

    if(volume > SOUND_VOLUME_MAX)
        volume = SOUND_VOLUME_MAX;

    channels[channel].volume = volume;
    update_output_volume(channel);
}

void sound_panning(u32 channel, i32 panning) {
    if(channel >= CHANNEL_COUNT)
        return;

    if(panning > +64) panning = +64;
    if(panning < -64) panning = -64;

    channels[channel].panning = panning;
    update_output_volume(channel);
}
