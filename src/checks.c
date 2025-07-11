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
#include "libsimplegba/base.h"

static_assert(true,   "value of true is incorrect");
static_assert(!false, "value of false is incorrect");

static_assert(sizeof(u8)  == 1, "size of u8 is incorrect");
static_assert(sizeof(u16) == 2, "size of u16 is incorrect");
static_assert(sizeof(u32) == 4, "size of u32 is incorrect");
static_assert(sizeof(i8)  == 1, "size of i8 is incorrect");
static_assert(sizeof(i16) == 2, "size of i16 is incorrect");
static_assert(sizeof(i32) == 4, "size of i32 is incorrect");

static_assert((u8)  U8_MAX  == U8_MAX,  "value of U8_MAX is too large");
static_assert((u16) U16_MAX == U16_MAX, "value of U16_MAX is too large");
static_assert((u32) U32_MAX == U32_MAX, "value of U32_MAX is too large");

static_assert((i8)  I8_MIN  == I8_MIN,  "value of I8_MIN is too large");
static_assert((i16) I16_MIN == I16_MIN, "value of I16_MIN is too large");
static_assert((i32) I32_MIN == I32_MIN, "value of I32_MIN is too large");

static_assert((i8)  I8_MAX  == I8_MAX,  "value of I8_MAX is too large");
static_assert((i16) I16_MAX == I16_MAX, "value of I16_MAX is too large");
static_assert((i32) I32_MAX == I32_MAX, "value of I32_MAX is too large");
