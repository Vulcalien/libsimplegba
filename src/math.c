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
#include "math.h"

i16 math_sin(u16 angle) {
    // val in range [0, MATH_PI * MATH_PI / 4]
    u32 val = (angle % MATH_PI) * (MATH_PI - (angle % MATH_PI));

    // normalize to [0, 0x4000]
    i16 result = val / ((MATH_PI * MATH_PI / 4) / 0x4000);

    // correct sign
    if(angle % (2 * MATH_PI) > MATH_PI)
        result = -result;

    return result;
}

i16 math_cos(u16 angle) {
    return math_sin(angle + MATH_PI / 2);
}
