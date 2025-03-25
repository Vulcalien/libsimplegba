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

#include <gba/system.h>
#include <gba/timer.h>

#define AUDIO_VOLUME_MAX 64

#define AUDIO_PANNING_MIN (-64)
#define AUDIO_PANNING_MAX (+64)

// DEBUG
#define AUDIO_CHANNEL_COUNT 2

extern void audio_init(void);
extern void audio_update(void);

extern i32 audio_play(i32 channel, const void *sound, u32 length);

extern void _audio_stop(i32 channel);
INLINE void audio_stop(i32 channel) {
    if(channel < 0) {
        for(u32 c = 0; c < AUDIO_CHANNEL_COUNT; c++)
            _audio_stop(c);
    } else if(channel < AUDIO_CHANNEL_COUNT) {
        _audio_stop(channel);
    }
}

extern void _audio_loop(i32 channel, u32 loop_length);
INLINE void audio_loop(i32 channel, u32 loop_length) {
    if(channel < 0) {
        for(u32 c = 0; c < AUDIO_CHANNEL_COUNT; c++)
            _audio_loop(c, loop_length);
    } else if(channel < AUDIO_CHANNEL_COUNT) {
        _audio_loop(channel, loop_length);
    }
}

extern void _audio_volume(i32 channel, u32 volume);
INLINE void audio_volume(i32 channel, u32 volume) {
    if(channel < 0) {
        for(u32 c = 0; c < AUDIO_CHANNEL_COUNT; c++)
            _audio_volume(c, volume);
    } else if(channel < AUDIO_CHANNEL_COUNT) {
        _audio_volume(channel, volume);
    }
}

extern void _audio_panning(i32 channel, i32 panning);
INLINE void audio_panning(i32 channel, i32 panning) {
    if(channel < 0) {
        for(u32 c = 0; c < AUDIO_CHANNEL_COUNT; c++)
            _audio_panning(c, panning);
    } else if(channel < AUDIO_CHANNEL_COUNT) {
        _audio_panning(channel, panning);
    }
}

INLINE void audio_sample_rate(u32 sample_rate) {
    if(sample_rate == 0)
        sample_rate = 16384;

    const u32 cycles_per_sample = SYSTEM_FREQUENCY / sample_rate;
    timer_start(TIMER0, cycles_per_sample);
}
