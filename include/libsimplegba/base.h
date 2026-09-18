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

#include "libsimplegba/misc/types.h"
#include "libsimplegba/misc/attributes.h"

#define BIT(n) (1 << (n))

#define static_assert _Static_assert

// Sections
#define SBSS_SECTION         __attribute__((section(".sbss")))
#define EWRAM_SECTION        __attribute__((section(".ewram")))
#define EWRAM_RODATA_SECTION __attribute__((section(".ewram.rodata")))
#define IWRAM_SECTION        __attribute__((section(".iwram")))
#define IWRAM_RODATA_SECTION __attribute__((section(".iwram.rodata")))
