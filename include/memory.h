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

#define memory_set memset
extern void *memory_set_8 (volatile void *dest, i32 byte, u32 n);
extern void *memory_set_16(volatile void *dest, i32 byte, u32 n);
extern void *memory_set_32(volatile void *dest, i32 byte, u32 n);

// TODO #define memory_copy memcpy
extern void *memory_copy_8 (volatile void *dest, volatile const void *src, u32 n);
extern void *memory_copy_16(volatile void *dest, volatile const void *src, u32 n);
extern void *memory_copy_32(volatile void *dest, volatile const void *src, u32 n);
