/* Copyright 2023-2025 Vulcalien
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

// Basic audio driver offering 2 independent channels, each with:
// - looping support
// - limited volume control (50%, 100%)
// - basic panning control (left, right, both)
//
// Each channel is mapped directly to a Direct Sound output, so the CPU
// usage remains very low.

#define DIRECT_SOUND_CONTROL *((vu16 *) 0x04000082)
#define MASTER_SOUND_CONTROL *((vu16 *) 0x04000084)

#define CHANNEL_COUNT 2

static const struct Output {
    vi8 *fifo;
    u32 dma;

    u8 enable_bits;
    struct {
        u16 reset;
        u16 volume;
    } bits;
} outputs[2] = {
    {
        .fifo = (vi8 *) 0x040000a0,
        .dma = DMA1,

        .enable_bits = 8,
        .bits = {
            .reset = BIT(11),
            .volume = BIT(2)
        }
    }, {
        .fifo = (vi8 *) 0x040000a4,
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
static NO_INLINE void restart_dma(i32 channel) {
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

static INLINE void update_enable_bits(i32 channel) {
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

THUMB
static i32 basic_play(i32 channel, const void *sound, u32 length) {
    // if sound length is zero, stop the channel
    if(length == 0) {
        audio_stop(channel);
        return channel;
    }

    struct Channel *channel_struct = &channels[channel];
    channel_struct->data = (const i8 *) sound;
    channel_struct->end  = (const i8 *) sound + length;

    restart_dma(channel);
    update_enable_bits(channel);

    // add unplayed samples back into the other channel's data pointer
    struct Channel *other_channel = &channels[channel ^ 1];
    if(other_channel->data)
        other_channel->data -= timer_get_counter(TIMER1);

    // reschedule the next IRQ
    schedule_timer1_irq();

    return channel;
}

THUMB
static void basic_stop(i32 channel) {
    channels[channel].data = NULL;

    update_enable_bits(channel);
    dma_stop(outputs[channel].dma);
}

THUMB
static void basic_loop(i32 channel, u32 loop_length) {
    channels[channel].loop_length = loop_length;
}

THUMB
static void basic_volume(i32 channel, u32 volume) {
    const u16 bit = outputs[channel].bits.volume;
    if(volume > AUDIO_VOLUME_MAX / 2) // 100%
        DIRECT_SOUND_CONTROL |= bit;
    else // 50%
        DIRECT_SOUND_CONTROL &= ~bit;
}

THUMB
static void basic_panning(i32 channel, i32 panning) {
    // audio only plays on one side if panning is at either extreme
    bool left  = (panning < AUDIO_PANNING_MAX);
    bool right = (panning > AUDIO_PANNING_MIN);
    channels[channel].directions = left << 1 | right;

    update_enable_bits(channel);
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
                basic_stop(c);
            }
        }
    }

    // schedule the next IRQ
    schedule_timer1_irq();
}

static void basic_init(void) {
    MASTER_SOUND_CONTROL = BIT(7);
    DIRECT_SOUND_CONTROL = 0;

    // configure Timer 0 and Timer 1
    timer_config(TIMER0, TIMER_PRESCALER_1);
    timer_config(TIMER1, TIMER_CASCADE);

    // enable Timer 1 interrupt
    interrupt_toggle(IRQ_TIMER1, true);
    interrupt_set_isr(IRQ_TIMER1, timer1_isr);

    // setup channels with default configuration
    for(u32 c = 0; c < CHANNEL_COUNT; c++) {
        basic_loop   (c, 0);
        basic_volume (c, AUDIO_VOLUME_MAX);
        basic_panning(c, 0);
    }

    // Set default sample rate. This also starts Timer 0
    audio_sample_rate(0);
}

THUMB
static void basic_update(void) {
    // nothing to do
}

static i32 basic_available_channel(void) {
    for(u32 c = 0; c < CHANNEL_COUNT; c++)
        if(!channels[c].data)
            return c;
    return -1;
}

const struct _AudioDriver _audio_driver_basic = {
    .init   = basic_init,
    .update = basic_update,

    .play = basic_play,
    .stop = basic_stop,

    .loop    = basic_loop,
    .volume  = basic_volume,
    .panning = basic_panning,

    .channel_count     = CHANNEL_COUNT,
    .available_channel = basic_available_channel
};
