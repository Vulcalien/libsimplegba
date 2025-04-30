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
#include <math.h>

IWRAM_SECTION
i32 math_sqrt(u32 x) {
    // set initial bounds so that every possible result is included
    u32 low = 0;
    u32 high = U16_MAX + 1;

    // try to shrink bounds using powers of two (up to 2^15)
    #pragma GCC unroll 0 // tell compiler not to unroll this loop
    for(u32 i = 4; i < 16; i++) {
        u32 guess = (1 << i);             // 2^i
        u32 guess_squared = (guess << i); // 2^i * 2^i

        // update bounds
        if(guess_squared <= x) {
            low = guess;
        } else {
            high = guess;
            break;
        }
    }

    // use binary search in range [low, high)
    while(high - low > 1) {
        const u32 mid = (low + high) / 2;
        const u32 mid_squared = mid * mid;

        // update bounds
        if(mid_squared <= x) low  = mid;
        if(mid_squared >= x) high = mid;
    }

    // return low = floor(sqrt(x))
    return low;
}
