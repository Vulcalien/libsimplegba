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
#pragma once

#include <base.h>

#include <gba/timer.h>

// This small module can be used for profiling.
// 'profiler_start' starts Timer 2 and Timer 3 in cascade.
// 'profiler_stop' stops the timers and returns an indicative (i.e. not
// 100% accurate) number of cycles elapsed since the last call to
// 'profiler_start'. The timers can count up to 256 seconds.

INLINE void profiler_start(void) {
    timer_config(TIMER2, TIMER_PRESCALER_1);
    timer_config(TIMER3, TIMER_CASCADE);

    // the timer in cascade (timer 3) should be started first
    timer_start(TIMER3, TIMER_COUNTER_MAX);
    timer_start(TIMER2, TIMER_COUNTER_MAX);
}

INLINE u32 profiler_stop(void) {
    timer_stop(TIMER2); // no need to also stop timer 3

    u32 t2 = TIMER_COUNTER_MAX - timer_get_counter(TIMER2);
    u32 t3 = TIMER_COUNTER_MAX - timer_get_counter(TIMER3);

    return t3 << 16 | t2;
}
