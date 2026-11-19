/* Copyright 2026 Vulcalien
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

#include "libsimplegba/base.h"

#include "libsimplegba/hw/system.h"
#include "libsimplegba/hw/timer.h"

// The bitrate range limits are chosen so that:
// - the number of cycles between transfers doesn't exceed the maximum
//   counter value of timers with a prescaler of 64
// - a baud rate of 57600 bps is enough to support all four GBAs
// - the CPU isn't overwhelmed by serial IRQs
#define LINKCABLE_BITRATE_MIN 32
#define LINKCABLE_BITRATE_MAX 4096
// TODO move bitrate limits to protocols

extern const struct LinkCableProtocol {
    void (*init)(void);

    i32 (*send)(i32 dest, const void *buffer, u32 n);
    i32 (*receive)(i32 *src, void *buffer, u32 n);
} *_linkcable_protocol;

INLINE void linkcable_init(const void *protocol) {
    _linkcable_protocol = (const struct LinkCableProtocol *) protocol;
    _linkcable_protocol->init();
}

INLINE void linkcable_speed(u32 bitrate) {
    if(bitrate == 0) {
        timer_stop(TIMER2);
        return;
    }

    if(bitrate < LINKCABLE_BITRATE_MIN)
        bitrate = LINKCABLE_BITRATE_MIN;
    if(bitrate > LINKCABLE_BITRATE_MAX)
        bitrate = LINKCABLE_BITRATE_MAX;

    const u32 cycles = (SYSTEM_FREQUENCY * 8 / 64) / bitrate;
    timer_config(TIMER2, TIMER_PRESCALER_64);
    timer_start(TIMER2, cycles);
}

INLINE i32 linkcable_send(i32 dest, const void *buffer, u32 n) {
    return _linkcable_protocol->send(dest, buffer, n);
}

INLINE i32 linkcable_receive(i32 *src, void *buffer, u32 n) {
    return _linkcable_protocol->receive(src, buffer, n);
}

INLINE i32 linkcable_broadcast(const void *buffer, u32 n) {
    return _linkcable_protocol->send(-1, buffer, n);
}

extern const struct LinkCableProtocol
    _linkcable_protocol_byte;

#define LINKCABLE_BYTE (&_linkcable_protocol_byte)
