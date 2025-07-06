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
extern void *memcpy(volatile void *dest, volatile const void *src, u32 n);
extern i32 memcmp(volatile const void *a, volatile const void *b, u32 n);

#define memory_set memset
extern void *memory_set_8 (volatile void *dest, i32 byte, u32 n);
extern void *memory_set_16(volatile void *dest, i32 byte, u32 n);
extern void *memory_set_32(volatile void *dest, i32 byte, u32 n);

#define memory_copy memcpy
extern void *memory_copy_8 (volatile void *dest, volatile const void *src, u32 n);
extern void *memory_copy_16(volatile void *dest, volatile const void *src, u32 n);
extern void *memory_copy_32(volatile void *dest, volatile const void *src, u32 n);

#define memory_compare memcmp
extern i32 memory_compare_8 (volatile const void *a, volatile const void *b, u32 n);
extern i32 memory_compare_16(volatile const void *a, volatile const void *b, u32 n);
extern i32 memory_compare_32(volatile const void *a, volatile const void *b, u32 n);

#define memory_clear(dest, n)    memory_set   ((dest), 0, (n))
#define memory_clear_8(dest, n)  memory_set_8 ((dest), 0, (n))
#define memory_clear_16(dest, n) memory_set_16((dest), 0, (n))
#define memory_clear_32(dest, n) memory_set_32((dest), 0, (n))
