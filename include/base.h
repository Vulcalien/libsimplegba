/* Copyright 2024 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

// Basic types and definitions used throughout the program.

// === Data Types ===

#define NULL ((void *) 0)

typedef _Bool bool;
#define true  1
#define false 0

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef   signed char   i8;
typedef   signed short  i16;
typedef   signed int    i32;

typedef volatile u8     vu8;
typedef volatile u16    vu16;
typedef volatile u32    vu32;
typedef volatile i8     vi8;
typedef volatile i16    vi16;
typedef volatile i32    vi32;

// Limits
#define  U8_MAX 255
#define U16_MAX 65535
#define U32_MAX 4294967295

#define  I8_MIN (-128)
#define I16_MIN (-32768)
#define I32_MIN (-2147483648)

#define  I8_MAX 127
#define I16_MAX 32767
#define I32_MAX 2147483647

// === Macros ===

#define BIT(n) (1 << (n))
#define BITMASK(n) ((1 << (n)) - 1)

// ===== ===== =====

#define static_assert _Static_assert

// Sections
#define SBSS_SECTION         __attribute__((section(".sbss")))
#define EWRAM_SECTION        __attribute__((section(".ewram")))
#define EWRAM_RODATA_SECTION __attribute__((section(".ewram.rodata")))
#define IWRAM_SECTION        __attribute__((section(".iwram")))
#define IWRAM_RODATA_SECTION __attribute__((section(".iwram.rodata")))

// Variable attributes
#define ALIGNED(x) __attribute__((aligned(x)))

// Function attributes
#define ARM   __attribute__((target("arm")))
#define THUMB __attribute__((target("thumb")))

#define INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline, noclone))

// Constants
#define CLOCK_FREQUENCY (16 * 1024 * 1024)
