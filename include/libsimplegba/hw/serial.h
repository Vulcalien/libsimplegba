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

extern const struct SerialDriver {
    i32 (*send)(const void *buffer, u32 n);
    i32 (*receive)(void *buffer, u32 n);
    i32 (*transfer)(const void *outbuf, void *inbuf, u32 n);
} *_serial_driver;

INLINE void serial_init(const void *driver) {
    _serial_driver = (const struct SerialDriver *) driver;
}

INLINE i32 serial_send(const void *buffer, u32 n) {
    return _serial_driver->send(buffer, n);
}

INLINE i32 serial_receive(void *buffer, u32 n) {
    return _serial_driver->receive(buffer, n);
}

INLINE i32 serial_transfer(const void *outbuf, void *inbuf, u32 n) {
    return _serial_driver->transfer(outbuf, inbuf, n);
}
