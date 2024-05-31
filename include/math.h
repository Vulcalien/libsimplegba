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

#include "base.h"

#define MATH_PI 0x8000

INLINE i32 math_min(i32 a, i32 b) {
    return (a < b ? a : b);
}

INLINE i32 math_max(i32 a, i32 b) {
    return (a > b ? a : b);
}

INLINE i32 math_abs(i32 val) {
    return (val < 0 ? -val : val);
}

INLINE i32 math_sign(i32 val) {
    if(val < 0)
        return -1;
    if(val > 0)
        return +1;
    return 0;
}

// Approximate 'sin' and 'cos' functions
//
// 'angle' in range [0, 0xffff]
// Output in range [-0x4000, 0x4000]
extern i16 math_sin(u16 angle);

INLINE i16 math_cos(u16 angle) {
    return math_sin(angle + MATH_PI / 2);
}
