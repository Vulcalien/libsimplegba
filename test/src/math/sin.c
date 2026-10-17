/* Copyright 2026 Vulcalien
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
#include "test.h"

TEST(math_sin_odd) {
    for(i32 i = 0; i < math_brad(180); i++)
        if(math_sin(i) != -math_sin(-i))
            return false;
    return true;
}

TEST(math_sin_axial_symmetry) {
    for(i32 i = 0; i < math_brad(90); i++)
        if(math_sin(i) != math_sin(math_brad(180) - i))
            return false;
    return true;
}

TEST(math_sin_periodic) {
    random_seed(0);
    for(i32 i = 0; i < math_brad(360); i++) {
        i32 k = random(257) - 128;
        i32 offset = k * math_brad(360);
        if(math_sin(i) != math_sin(i + offset))
            return false;
    }
    return true;
}
