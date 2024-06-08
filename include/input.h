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

#define _INPUT_KEY_INPUT   *((vu16 *) 0x04000130)
#define _INPUT_KEY_CONTROL *((vu16 *) 0x04000132)

#define _INPUT_IRQ_BIT BIT(14)

extern u16 _input_is_down;
extern u16 _input_was_down;

INLINE void input_tick(void) {
    _input_was_down = _input_is_down;
    _input_is_down  = _INPUT_KEY_INPUT ^ 0x3ff;
}

INLINE bool input_down(u16 key) {
    bool is_down = (_input_is_down & key);

    return is_down;
}

INLINE bool input_pressed(u16 key) {
    bool is_down  = (_input_is_down  & key);
    bool was_down = (_input_was_down & key);

    return is_down && !was_down;
}

INLINE bool input_released(u16 key) {
    bool is_down  = (_input_is_down  & key);
    bool was_down = (_input_was_down & key);

    return !is_down && was_down;
}

// all_keys: if true, all specified keys must be pressed at the same
//           time to raise the interrupt; if false, any of the specified
//           keys will raise the interrupt.
// keys: list of keys joined using logical OR (e.g. KEY_A | KEY_START).
INLINE void input_irq_config(bool all_keys, u16 keys) {
    // clear all bits, except IRQ enable
    u16 val = _INPUT_KEY_CONTROL & _INPUT_IRQ_BIT;

    val |= keys     << 0 |
           all_keys << 15;
    _INPUT_KEY_CONTROL = val;
}

#undef _INPUT_KEY_INPUT
#undef _INPUT_KEY_CONTROL

#undef _INPUT_IRQ_BIT
