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

// This function calculates the integral part (i.e. floor) of the square
// root of any unsigned 32-bit integer. It is based on the Binary Search
// algorithm. The initial upper bound is either the argument or U16_MAX
// (i.e. 65535, the biggest integer that squared gives a 32-bit number).

IWRAM_SECTION
u16 math_sqrt(u32 val) {
    if(val == 0)
        return 0;

    // If 'val' is greater than 'U16_MAX ^ 2', 32 bits are not enough to
    // contain intermediate results: return the only possible solution.
    if(val >= (u32) U16_MAX * U16_MAX)
        return U16_MAX;

    // Initialize the search boundaries. Note: 'high' cannot be
    // calculated using 'math_min' because of different signedness.
    u32 low  = 1;
    u32 high = (val < U16_MAX ? val : U16_MAX);

    // Iterate until 'low' and 'high' are equal or differ by one.
    while(high - low > 1) {
        const u32 mid = (low + high) / 2;
        const u32 mid_squared = mid * mid;

        if(mid_squared <= val) low  = mid;
        if(mid_squared >= val) high = mid;
    }

    // Return the lower bound: if 'low != high' then the 'val' is not a
    // perfect square and 'low = floor(sqrt(val))'.
    return low;
}
