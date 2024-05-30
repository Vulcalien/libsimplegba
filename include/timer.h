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

#include "base.h"

// This is how timers are considered to work by this driver.
//
// Each timer has its own counter, which can have values in range
// [1, TIMER_COUNTER_MAX]. When started, a timer is given a counter
// value. Timers decrement their counter when ticking until it reaches
// zero. When that happens, the timer immediately loads the counter's
// next value (the same value specified when started, unless changed).
//
// 'prescaler' is the number of CPU cycles it takes a timer to tick.
// If cascade=1, Timer X will tick only when the counter of Timer X-1
// reaches zero and 'prescaler' will be ignored.
// If irq=1, an interrupt request is raised when the counter
// reaches zero.

#define TIMER_COUNT (4)

#define TIMER0 (0)
#define TIMER1 (1)
#define TIMER2 (2)
#define TIMER3 (3)

#define TIMER_COUNTER_MAX (U16_MAX + 1)

#define _TIMER_GET_CONTROL(id) ((vu16 *) (0x04000102 + id * 4))
#define _TIMER_GET_RELOAD(id)  ((vu16 *) (0x04000100 + id * 4))

struct Timer {
    u16 prescaler : 2; // 0=1, 1=64, 2=256, 3=1024 (number of cycles)
    u16 cascade   : 1; // 0=disable, 1=enable
    u16 irq       : 1; // 0=disable, 1=enable
};

INLINE void timer_config(u32 id, const struct Timer *config) {
    if(id >= TIMER_COUNT)
        return;

    vu16 *control = _TIMER_GET_CONTROL(id);
    if(config) {
        *control = config->prescaler << 0 |
                   config->cascade   << 2 |
                   config->irq       << 6;
    } else {
        *control = 0;
    }
}

INLINE void timer_start(u32 id, u32 ticks) {
    if(id >= TIMER_COUNT)
        return;

    vu16 *reload = _TIMER_GET_RELOAD(id);
    *reload = (TIMER_COUNTER_MAX - ticks) & 0xffff;

    vu16 *control = _TIMER_GET_CONTROL(id);
    *control |= (1 << 7);
}

INLINE void timer_stop(u32 id) {
    if(id >= TIMER_COUNT)
        return;

    vu16 *control = _TIMER_GET_CONTROL(id);
    *control &= ~(1 << 7);
}

INLINE void timer_restart(u32 id, u32 ticks) {
    timer_stop(id);
    timer_start(id, ticks);
}

// returns values in range [1, TIMER_COUNTER_MAX]
INLINE u32 timer_get_counter(u32 id) {
    if(id >= TIMER_COUNT)
        return 0;

    vu16 *counter = _TIMER_GET_RELOAD(id);
    return TIMER_COUNTER_MAX - *counter;
}

#undef _TIMER_GET_CONTROL
#undef _TIMER_GET_RELOAD
