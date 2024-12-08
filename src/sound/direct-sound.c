/* Copyright 2023-2024 Vulcalien
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

// Basic audio driver offering 2 independent channels, each with:
// - looping support
// - limited volume control (50%, 100%)
// - basic panning control (left, right, both)
//
// Each channel is mapped directly to an output, so that the CPU usage
// is very low.

#define CHANNEL_COUNT 2

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)
#define MASTER_SOUND_CONTROL *((vu16 *) 0x04000084)

static const struct Output {
    vu32 *fifo;
    u32 dma;

    u8 enable_bits;
    struct {
        u16 reset;
        u16 volume;
    } bits;
} outputs[2] = {
    {
        .fifo = (vu32 *) 0x040000a0,
        .dma = DMA1,

        .enable_bits = 8,
        .bits = {
            .reset = BIT(11),
            .volume = BIT(2)
        }
    }, {
        .fifo = (vu32 *) 0x040000a4,
        .dma = DMA2,

        .enable_bits = 12,
        .bits = {
            .reset = BIT(15),
            .volume = BIT(3)
        }
    }
};

static struct Channel {
    const i8 *data; // NULL if channel is inactive
    const i8 *end;

    u32 loop_length;
    u8 directions;
} channels[CHANNEL_COUNT];

IWRAM_SECTION
static void restart_dma(u32 channel) {
    // reset FIFO
    DIRECT_SOUND_CONTROL |= outputs[channel].bits.reset;

    // reset DMA
    dma_config(outputs[channel].dma, &(struct DMA) {
        .start_timing = DMA_START_SPECIAL,
        .repeat = true
    });
    dma_transfer(
        outputs[channel].dma,
        outputs[channel].fifo,
        channels[channel].data, 0
    );
}

static INLINE void update_enable_bits(u32 channel) {
    const u32 enable_bits = outputs[channel].enable_bits;
    struct Channel *channel_struct = &channels[channel];

    // Clear enable bits. It is important to always clear the bits,
    // even before setting them, in case the directions changed.
    DIRECT_SOUND_CONTROL &= ~(3 << enable_bits);

    const u32 directions = channel_struct->directions;

    // if playing, set enable bits
    if(channel_struct->data)
        DIRECT_SOUND_CONTROL |= (directions << enable_bits);
}

static INLINE void schedule_timer1_irq(void) {
    // determine how many samples should be played before stopping
    u32 next_stop = TIMER_COUNTER_MAX;
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        struct Channel *channel = &channels[c];
        if(!channel->data)
            continue;

        const u32 remaining = channel->end - channel->data;
        if(remaining < next_stop)
            next_stop = remaining;
    }

    // add number of samples to be played to data pointers of channels
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        struct Channel *channel = &channels[c];
        if(!channel->data)
            continue;

        channel->data += next_stop;
    }

    // restart Timer 1
    timer_restart(TIMER1, next_stop);
}

IWRAM_SECTION
static void timer1_isr(void) {
    // stop or loop the channels
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        struct Channel *channel = &channels[c];
        if(!channel->data)
            continue;

        if(channel->data >= channel->end) {
            if(channel->loop_length != 0) {
                channel->data = channel->end - channel->loop_length;
                restart_dma(c);
            } else {
                sound_stop(c);
            }
        }
    }

    // schedule the next IRQ
    schedule_timer1_irq();
}

void sound_init(void) {
    MASTER_SOUND_CONTROL = BIT(7); // enable sound
    DIRECT_SOUND_CONTROL = 0; // use Timer 0 for both DMA channels

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, TIMER_PRESCALER_1);
    timer_config(TIMER1, TIMER_CASCADE);

    // enable Timer 1 interrupt
    interrupt_toggle(IRQ_TIMER1, true);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // setup channels with default configuration
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        sound_loop(c, 0);
        sound_volume(c, SOUND_VOLUME_MAX);
        sound_panning(c, 0);
    }

    // Set default sample rate. This also starts Timer 0
    sound_sample_rate(0);
}

void sound_play(u32 channel, const u8 *sound, u32 length) {
    if(channel >= CHANNEL_COUNT)
        return;

    if(length == 0) {
        sound_stop(channel);
        return;
    }

    struct Channel *channel_struct = &channels[channel];
    channel_struct->data = (i8 *) sound;
    channel_struct->end  = (i8 *) sound + length;

    restart_dma(channel);
    update_enable_bits(channel);

    // add unplayed samples back into the other channel's data pointer
    struct Channel *other_channel = &channels[channel ^ 1];
    if(other_channel->data)
        other_channel->data -= timer_get_counter(TIMER1);

    // reschedule the next IRQ
    schedule_timer1_irq();
}

void sound_stop(u32 channel) {
    if(channel >= CHANNEL_COUNT)
        return;

    channels[channel].data = NULL;

    update_enable_bits(channel);
    dma_stop(outputs[channel].dma);
}

void sound_loop(u32 channel, u32 loop_length) {
    if(channel >= CHANNEL_COUNT)
        return;

    channels[channel].loop_length = loop_length;
}

void sound_volume(u32 channel, u32 volume) {
    if(channel >= CHANNEL_COUNT)
        return;

    const u16 bit = outputs[channel].bits.volume;
    if(volume > SOUND_VOLUME_MAX / 2) // 100%
        DIRECT_SOUND_CONTROL |= bit;
    else // 50%
        DIRECT_SOUND_CONTROL &= ~bit;
}

void sound_panning(u32 channel, i32 panning) {
    if(channel >= CHANNEL_COUNT)
        return;

    // audio plays in one side only if panning is at either extreme
    bool left  = (panning < SOUND_PANNING_MAX);
    bool right = (panning > SOUND_PANNING_MIN);
    channels[channel].directions = left << 1 | right;

    update_enable_bits(channel);
}
