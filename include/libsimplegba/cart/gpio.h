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
#pragma once

#include "libsimplegba/base.h"

INLINE void gpio_toggle(bool enable) {
    vu16 *control = (vu16 *) 0x080000c8;
    *control = enable;
}

// 0=input      Cartridge ->    GBA
// 1=output        GBA    -> Cartridge
INLINE void gpio_config(u32 directions) {
    vu16 *pin_control = (vu16 *) 0x080000c6;
    *pin_control = directions;
}

INLINE u16 gpio_read(void) {
    vu16 *pins = (vu16 *) 0x080000C4;
    return *pins;
}

INLINE void gpio_write(u16 data) {
    vu16 *pins = (vu16 *) 0x080000C4;
    *pins = data;
}
