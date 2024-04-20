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
#include "input.h"

#define KEY_INPUT *((vu16 *) 0x04000130)

u16 input_keys_is_down  = 0;
u16 input_keys_was_down = 0;

void input_tick(void) {
    input_keys_was_down = input_keys_is_down;
    input_keys_is_down  = KEY_INPUT ^ 0x3ff;
}
