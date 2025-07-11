/* Copyright 2025 Vulcalien
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
#include "libsimplegba/hw/input.h"

#define KEY_INPUT   *((vu16 *) 0x04000130)
#define KEY_CONTROL *((vu16 *) 0x04000132)

u16 _input_is_down;
u16 _input_was_down;
u16 _input_repeated;

u8 _input_repeat_delay;
u8 _input_repeat_interval;

THUMB
void input_update(void) {
    static u8 time_down[10];

    _input_was_down = _input_is_down;
    _input_is_down  = KEY_INPUT ^ 0x3ff;

    // update repeated status
    # pragma GCC unroll 0
    for(u32 i = 0; i < 10; i++) {
        const u32 key = BIT(i);
        if(input_down(key))
            time_down[i]++;
        else
            time_down[i] = 0;

        if(time_down[i] == 1 || time_down[i] > _input_repeat_delay)
            _input_repeated |= key;
        else
            _input_repeated &= ~key;

        if(time_down[i] > _input_repeat_delay)
            time_down[i] -= _input_repeat_interval;
    }
}
