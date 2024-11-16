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

extern void *memset(volatile void *dest, i32 byte, u32 n);

// alignment-specific versions of 'memset'
extern void *memset8 (volatile void *dest, i32 byte, u32 n);
extern void *memset16(volatile void *dest, i32 byte, u32 n);
extern void *memset32(volatile void *dest, i32 byte, u32 n);

// alignment-specific versions of 'memcpy'
extern void *memcpy8 (volatile void *dest, volatile const void *src, u32 n);
extern void *memcpy16(volatile void *dest, volatile const void *src, u32 n);
extern void *memcpy32(volatile void *dest, volatile const void *src, u32 n);
