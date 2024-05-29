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
#pragma once

#include "base.h"

#include "timer.h"

extern void sound_init(void);

// === Direct Sound ===

typedef bool sound_dma_Channel;
#define SOUND_DMA_A (0)
#define SOUND_DMA_B (1)

#define SOUND_DMA_PLAY(sound, loop, channel)\
    sound_dma_play((sound), sizeof(sound), (loop), (channel))

extern void sound_dma_play(const u8 *sound, u32 length, bool loop,
                           sound_dma_Channel channel);
extern void sound_dma_stop(sound_dma_Channel channel);

// volume: 0=50%, 1=100% (default=100%)
extern void sound_dma_volume(sound_dma_Channel channel, u32 volume);

INLINE void sound_dma_sample_rate(u32 sample_rate) {
    if(sample_rate == 0)
        sample_rate = 16384;

    const u32 cycles_per_sample = CLOCK_FREQUENCY / sample_rate;
    timer_start(TIMER0, cycles_per_sample);
}

// ===== ===== =====
