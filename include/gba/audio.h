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
#pragma once

#include <base.h>

#include <gba/system.h>
#include <gba/timer.h>

#define AUDIO_VOLUME_MAX 64

#define AUDIO_PANNING_MIN (-64)
#define AUDIO_PANNING_MAX (+64)

extern const struct _AudioDriver {
    void (*init)(void);
    void (*update)(void);

    i32  (*play)(i32 channel, const void *sound, u32 length);
    void (*stop)(i32 channel);

    void (*pause)(i32 channel);
    void (*resume)(i32 channel);

    void (*loop)(i32 channel, u32 loop_length);
    void (*volume)(i32 channel, u32 volume);
    void (*panning)(i32 channel, i32 panning);

    // internal use
    u32 channel_count;
    i32 (*available_channel)(void); // returns -1 if none available
} *_audio_driver;

INLINE void audio_init(const void *driver) {
    _audio_driver = (const struct _AudioDriver *) driver;
    _audio_driver->init();
}

INLINE void audio_update(void) {
    _audio_driver->update();
}

INLINE i32 audio_play(i32 channel, const void *sound, u32 length) {
    if(channel < 0)
        channel = _audio_driver->available_channel();

    if(channel < 0 || channel >= _audio_driver->channel_count)
        return -1;

    return _audio_driver->play(channel, sound, length);
}

INLINE void audio_stop(i32 channel) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->stop(c);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->stop(channel);
    }
}

INLINE void audio_pause(i32 channel) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->pause(c);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->pause(channel);
    }
}

INLINE void audio_resume(i32 channel) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->resume(c);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->resume(channel);
    }
}

INLINE void audio_loop(i32 channel, u32 loop_length) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->loop(c, loop_length);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->loop(channel, loop_length);
    }
}

INLINE void audio_volume(i32 channel, u32 volume) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->volume(c, volume);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->volume(channel, volume);
    }
}

INLINE void audio_panning(i32 channel, i32 panning) {
    if(channel < 0) {
        for(u32 c = 0; c < _audio_driver->channel_count; c++)
            _audio_driver->panning(c, panning);
    } else if(channel < _audio_driver->channel_count) {
        _audio_driver->panning(channel, panning);
    }
}

INLINE void audio_sample_rate(u32 sample_rate) {
    if(sample_rate == 0)
        sample_rate = 16384;

    const u32 cycles_per_sample = SYSTEM_FREQUENCY / sample_rate;
    timer_start(TIMER0, cycles_per_sample);
}

extern const struct _AudioDriver
    _audio_driver_basic,
    _audio_driver_mixer;

#define AUDIO_BASIC (&_audio_driver_basic)
#define AUDIO_MIXER (&_audio_driver_mixer)
