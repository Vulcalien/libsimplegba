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
#pragma once

#include <base.h>

#include <gba/timer.h>

#define AUDIO_VOLUME_MAX 64

#define AUDIO_PANNING_MIN (-64)
#define AUDIO_PANNING_MAX (+64)

extern void audio_init(void);

extern void audio_play(u32 channel, const u8 *sound, u32 length);
extern void audio_stop(u32 channel);

extern void audio_loop(u32 channel, u32 loop_length);
extern void audio_volume(u32 channel, u32 volume);
extern void audio_panning(u32 channel, i32 panning);

INLINE void audio_sample_rate(u32 sample_rate) {
    if(sample_rate == 0)
        sample_rate = 16384;

    const u32 cycles_per_sample = CLOCK_FREQUENCY / sample_rate;
    timer_start(TIMER0, cycles_per_sample);
}
