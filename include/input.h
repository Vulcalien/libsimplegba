/* Copyright 2023 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "base.h"

#define KEY_A      BIT(0)
#define KEY_B      BIT(1)
#define KEY_SELECT BIT(2)
#define KEY_START  BIT(3)

#define KEY_RIGHT  BIT(4)
#define KEY_LEFT   BIT(5)
#define KEY_UP     BIT(6)
#define KEY_DOWN   BIT(7)

#define KEY_R      BIT(8)
#define KEY_L      BIT(9)

extern u16 input_keys_is_down;
extern u16 input_keys_was_down;

extern void input_tick(void);

INLINE bool input_down(u16 key) {
    bool is_down = (input_keys_is_down & key);

    return is_down;
}

INLINE bool input_pressed(u16 key) {
    bool is_down  = (input_keys_is_down  & key);
    bool was_down = (input_keys_was_down & key);

    return is_down && !was_down;
}

INLINE bool input_released(u16 key) {
    bool is_down  = (input_keys_is_down  & key);
    bool was_down = (input_keys_was_down & key);

    return !is_down && was_down;
}
