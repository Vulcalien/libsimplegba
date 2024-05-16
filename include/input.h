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

#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)

#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)

#define KEY_R      (1 << 8)
#define KEY_L      (1 << 9)

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
