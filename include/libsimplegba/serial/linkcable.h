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

extern void linkcable_init(void);
extern void linkcable_update(void);

extern i32 linkcable_send(i32 dest, const void *buffer, u32 n);

INLINE i32 linkcable_broadcast(const void *buffer, u32 n) {
    return linkcable_send(-1, buffer, n);
}

extern i32 linkcable_receive(i32 *src, void *buffer, u32 n);
