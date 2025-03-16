/* Copyright 2024-2025 Vulcalien
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

INLINE i32 math_min(i32 a, i32 b) {
    return (a < b ? a : b);
}

INLINE i32 math_max(i32 a, i32 b) {
    return (a > b ? a : b);
}

INLINE i32 math_abs(i32 x) {
    return (x < 0 ? -x : x);
}

INLINE i32 math_sign(i32 x) {
    if(x < 0)
        return -1;
    if(x > 0)
        return +1;
    return 0;
}

INLINE i32 math_pow(i32 base, u32 exp) {
    i32 result = 1;
    while(exp > 0) {
        if(exp & 1)
            result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

extern i32 math_sqrt(u32 x);

INLINE i32 math_clip(i32 x, i32 min, i32 max) {
    if(x < min)
        return min;
    if(x > max)
        return max;
    return x;
}

INLINE i32 math_digits(i32 x, u32 base) {
    i32 result = 0;
    do {
        result++;
        x /= (i32) base;
    } while(x != 0);
    return result;
}

// === Trigonometry ===

INLINE i32 math_brad(i32 angle) {
    return angle * 0x2000 / 45;
}

extern i32 math_sin(i32 angle);

INLINE i32 math_cos(i32 angle) {
    return math_sin(angle + math_brad(90));
}

extern i32 math_tan(i32 angle);

extern i32 math_asin(i32 x);

INLINE i32 math_acos(i32 x) {
    return math_brad(90) - math_asin(x);
}
