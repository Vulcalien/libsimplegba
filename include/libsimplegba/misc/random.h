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

#include "libsimplegba/base.h"

#define RANDOM_MAX 65535

extern u32 _random_seed;

// 'bound' must be in range [1, RANDOM_MAX + 1]
INLINE i32 random(u32 bound) {
    // source (paper):
    //   Computationally easy, spectrally good multipliers for
    //   congruential pseudorandom number generators
    // by Guy Steele and Sebastiano Vigna
    const u32 multiplier = 0x915f77f5;

    _random_seed = _random_seed * multiplier + 1;
    return (_random_seed >> 16) * bound / (RANDOM_MAX + 1);
}

// 'seed' can be any unsigned 32-bit integer.
// The previously stored seed value is returned.
INLINE u32 random_seed(u32 seed) {
    u32 old = _random_seed;
    _random_seed = seed;
    return old;
}
