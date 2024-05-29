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
    struct {
        vu32 *source;
        vu32 *dest;
        vu16 *control;
    } dma;
} channels[2] = {
    // Channel A
    {
        .dma = { DMA1_SOURCE, DMA1_DEST, DMA1_CONTROL }
    },

    // Channel B
    {
        .dma = { DMA2_SOURCE, DMA2_DEST, DMA2_CONTROL }
    }
};

static u8 panning[2];

static struct SoundData {
    const u8 *sound;
    u32 length;
    bool loop;

    bool playing;
    u32 remaining;
} sound_data[2];

static inline void start_sound(const u8 *sound, u32 length, bool loop,
                               sound_dma_Channel channel) {
    const struct Channel *direct_channel = &channels[channel];

    // reset channel FIFO
    if(channel == SOUND_DMA_A)
        DIRECT_SOUND_CONTROL |= (1 << 11);
    else
        DIRECT_SOUND_CONTROL |= (1 << 15);

    // reset DMA
    u16 dma_control = 2 << 5  | // Dest address control (2 = Fixed)
                      1 << 9  | // DMA repeat
                      1 << 10 | // Transfer type (1 = 32bit)
                      3 << 12 | // Start timing (3 = Sound FIFO)
                      1 << 15;  // DMA enable

    vu32 *fifo = (channel == SOUND_DMA_A ? FIFO_A : FIFO_B);

    *(direct_channel->dma.source)  = (u32) sound;
    *(direct_channel->dma.dest)    = (u32) fifo;
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

static inline void update_enable_bits(sound_dma_Channel channel) {
    const u32 enable_bits = (channel == SOUND_DMA_A ? 8 : 12);
    struct SoundData *data = &sound_data[channel];

    // clear enable bits
    DIRECT_SOUND_CONTROL &= ~(3 << enable_bits);

    // if playing, set enable bits
    if(data->playing)
        DIRECT_SOUND_CONTROL |= (panning[channel] << enable_bits);
}

void sound_dma_play(const u8 *sound, u32 length, bool loop,
                    sound_dma_Channel channel) {
    if(length == 0)
        return;

    start_sound(sound, length, loop, channel);
    update_enable_bits(channel);

    // add the samples that were not played back into the other
    // channel's count of remaining samples
    struct SoundData *other_data = &sound_data[channel ^ 1];
    other_data->remaining += timer_get_counter(TIMER1);

    // reschedule the next IRQ
    schedule_next_irq();
}

void sound_dma_stop(sound_dma_Channel channel) {
    const struct Channel *direct_channel = &channels[channel];
    struct SoundData *data = &sound_data[channel];

    data->playing = false;

    *(direct_channel->dma.control) = 0;
    update_enable_bits(channel);
}

void sound_dma_volume(sound_dma_Channel channel, u32 volume) {
    const u32 volume_bit = (channel == SOUND_DMA_A ? 2 : 3);

    if(volume != 0) // 100%
        DIRECT_SOUND_CONTROL |= (1 << volume_bit);
    else // 50%
        DIRECT_SOUND_CONTROL &= ~(1 << volume_bit);
}

void sound_dma_panning(sound_dma_Channel channel,
                       bool left, bool right) {
    panning[channel] = left << 1 | right;
    update_enable_bits(channel);
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
                sound_dma_stop(channel);
        }
    }

    // reschedule the next IRQ
    schedule_next_irq();
}

void sound_dma_init(void) {
    DIRECT_SOUND_CONTROL = 0 << 10 | // Channel A Timer (0 = Timer 0)
                           0 << 14;  // Channel B Timer (0 = Timer 0)

    // enable Timer 1 IRQ
    interrupt_enable(IRQ_TIMER1);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, NULL);
    timer_config(TIMER1, &(struct Timer) { .cascade = 1, .irq = 1 });

    // set volume to 100% on both channels
    sound_dma_volume(SOUND_DMA_A, 1);
    sound_dma_volume(SOUND_DMA_B, 1);

    // enable left and right on both channels
    sound_dma_panning(SOUND_DMA_A, true, true);
    sound_dma_panning(SOUND_DMA_B, true, true);

    // Set sample rate to the default value. This also starts Timer 0.
    sound_dma_sample_rate(0);
}
