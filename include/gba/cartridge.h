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
#pragma once

#include <base.h>

struct CartridgeHeader {
    u32 entry_point;
    u8  logo[156];

    char game_title[12];
    char game_code[4];
    char maker_code[2];

    u8 fixed_value; // must be 0x96

    u8 main_unit_code;
    u8 device_type;

    u8 reserved_1[7];

    u8 software_version;
    u8 checksum;

    u8 reserved_2[2];
};

#define CARTRIDGE_HEADER \
    ((volatile const struct CartridgeHeader *) 0x08000000)

INLINE bool cartridge_missing(void) {
    return CARTRIDGE_HEADER->fixed_value != 0x96;
}

// === GPIO ===

// Some cartridges offer access to extra devices (RTC, solar sensor...)
// through a 4-bit GPIO port. Each pin can be configured as input or
// output.

INLINE void cartridge_gpio_toggle(bool enable) {
    vu16 *control = (vu16 *) 0x080000c8;
    *control = enable;
}

// 0=input (Cartridge to GBA), 1=output (GBA to Cartridge)
INLINE void cartridge_gpio_config(u32 directions) {
    vu16 *pin_control = (vu16 *) 0x080000c6;
    *pin_control = directions;
}

INLINE u16 cartridge_gpio_read(void) {
    vu16 *pins = (vu16 *) 0x080000C4;
    return *pins;
}

INLINE void cartridge_gpio_write(u16 data) {
    vu16 *pins = (vu16 *) 0x080000C4;
    *pins = data;
}
