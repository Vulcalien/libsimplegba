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
