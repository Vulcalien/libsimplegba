/* Copyright 2023-2024 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "sound.h"

#include "sound_p.h"

#include "interrupt.h"
#include "timer.h"

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)

#define FIFO_A ((vu32 *) 0x040000a0)
#define FIFO_B ((vu32 *) 0x040000a4)

// DMA
#define DMA1_SOURCE  ((vu32 *) 0x040000bc)
#define DMA1_DEST    ((vu32 *) 0x040000c0)
#define DMA1_CONTROL ((vu16 *) 0x040000c6)

#define DMA2_SOURCE  ((vu32 *) 0x040000c8)
#define DMA2_DEST    ((vu32 *) 0x040000cc)
#define DMA2_CONTROL ((vu16 *) 0x040000d2)

static const struct Channel {
    vu32 *fifo;

    struct {
        vu32 *source;
        vu32 *dest;
        vu16 *control;
    } dma;

    struct {
        bool left;
        bool right;
    } outputs;
} channels[2] = {
    // Channel A
    {
        .fifo = FIFO_A,
        .dma = { DMA1_SOURCE, DMA1_DEST, DMA1_CONTROL },
        .outputs = { true, true }
    },

    // Channel B
    {
        .fifo = FIFO_B,
        .dma = { DMA2_SOURCE, DMA2_DEST, DMA2_CONTROL },
        .outputs = { true, true }
    }
};

static struct SoundData {
    const u8 *sound;
    u32 length;
    bool loop;

    bool playing;
    u32 remaining;
} sound_data[2];

static inline void start_sound(const u8 *sound, u32 length, bool loop,
                               bool channel) {
    const struct Channel *direct_channel = &channels[channel];

    // reset channel FIFO
    if(channel == SOUND_CHANNEL_A)
        DIRECT_SOUND_CONTROL |= (1 << 11);
    else
        DIRECT_SOUND_CONTROL |= (1 << 15);

    // reset DMA
    u16 dma_control = 2 << 5  | // Dest address control (2 = Fixed)
                      1 << 9  | // DMA repeat
                      1 << 10 | // Transfer type (1 = 32bit)
                      3 << 12 | // Start timing (3 = Sound FIFO)
                      1 << 15;  // DMA enable

    *(direct_channel->dma.source)  = (u32) sound;
    *(direct_channel->dma.dest)    = (u32) direct_channel->fifo;
    *(direct_channel->dma.control) = 0;
    *(direct_channel->dma.control) = dma_control;

    // update the channel's sound_data
    sound_data[channel] = (struct SoundData) {
        .sound  = sound,
        .length = length,
        .loop   = loop,

        .playing   = true,
        .remaining = length
    };
}

// schedule the next Timer 1 IRQ
static inline void schedule_next_irq(void) {
    // determine how many samples should be played before stopping
    u32 next_stop = U16_MAX;
    for(u32 channel = 0; channel < 2; channel++) {
        struct SoundData *data = &sound_data[channel];

        if(data->playing && data->remaining < next_stop)
            next_stop = data->remaining;
    }

    // decrease the count of remaining samples
    for(u32 channel = 0; channel < 2; channel++) {
        struct SoundData *data = &sound_data[channel];

        // Checking if the channel is playing is unnecessary: if the
        // channel is not playing, this will have no effect.
        data->remaining -= next_stop;
    }

    // restart Timer 1
    timer_restart(TIMER1, next_stop);
}

static inline void set_channel_outputs(bool channel, bool enable) {
    const struct Channel *direct_channel = &channels[channel];

    u32 bits = (channel == SOUND_CHANNEL_A ? 8 : 12);
    u32 val = direct_channel->outputs.left << 1 |
              direct_channel->outputs.right;

    if(enable)
        DIRECT_SOUND_CONTROL |= (val << bits);
    else
        DIRECT_SOUND_CONTROL &= ~(val << bits);
}

void sound_play(const u8 *sound, u32 length, bool loop,
                bool channel) {
    if(length == 0)
        return;

    start_sound(sound, length, loop, channel);
    set_channel_outputs(channel, true);

    // add the samples that were not played back into the other
    // channel's count of remaining samples
    struct SoundData *other_data = &sound_data[channel ^ 1];
    other_data->remaining += timer_get_counter(TIMER1);

    // reschedule the next IRQ
    schedule_next_irq();
}

void sound_stop(bool channel) {
    const struct Channel *direct_channel = &channels[channel];
    struct SoundData *data = &sound_data[channel];

    *(direct_channel->dma.control) = 0;
    set_channel_outputs(channel, false);

    data->playing = false;
}

IWRAM_SECTION
static void timer1_isr(void) {
    // stop or loop the channels
    for(u32 channel = 0; channel < 2; channel++) {
        struct SoundData *data = &sound_data[channel];

        if(data->playing && data->remaining == 0) {
            if(data->loop)
                start_sound(data->sound, data->length, true, channel);
            else
                sound_stop(channel);
        }
    }

    // reschedule the next IRQ
    schedule_next_irq();
}

void sound_direct_init(void) {
    DIRECT_SOUND_CONTROL = 1 << 2  | // Channel A Volume (1 = 100%)
                           1 << 3  | // Channel B Volume (1 = 100%)
                           0 << 10 | // Channel A Timer (0 = Timer 0)
                           0 << 14;  // Channel B Timer (0 = Timer 0)

    // enable Timer 1 IRQ
    interrupt_enable(IRQ_TIMER1);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, NULL);
    timer_config(TIMER1, &(struct Timer) { .cascade = 1, .irq = 1 });

    // Set sample rate to the default: 16.384 KHz.
    // This operation also starts Timer 0.
    sound_set_sample_rate(16384);
}
