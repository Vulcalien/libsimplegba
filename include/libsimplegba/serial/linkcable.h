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

extern const struct LinkCableProtocol {
    void (*init)(void);

    i32 (*send)(i32 dest, const void *buffer, u32 n);
    i32 (*receive)(i32 *src, void *buffer, u32 n);
} *_linkcable_protocol;

INLINE void linkcable_init(const void *protocol) {
    _linkcable_protocol = (const struct LinkCableProtocol *) protocol;
    _linkcable_protocol->init();
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
