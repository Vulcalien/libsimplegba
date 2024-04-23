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

#include "base.h"

#define BACKGROUND_COUNT (4)

#define BG0 (0)
#define BG1 (1)
#define BG2 (2)
#define BG3 (3)

#define BG0_CONTROL ((vu16 *) 0x04000008)
#define BG1_CONTROL ((vu16 *) 0x0400000a)
#define BG2_CONTROL ((vu16 *) 0x0400000c)
#define BG3_CONTROL ((vu16 *) 0x0400000e)

#define BG0_XOFFSET ((vu16 *) 0x04000010)
#define BG0_YOFFSET ((vu16 *) 0x04000012)

#define BG1_XOFFSET ((vu16 *) 0x04000014)
#define BG1_YOFFSET ((vu16 *) 0x04000016)

#define BG2_XOFFSET ((vu16 *) 0x04000018)
#define BG2_YOFFSET ((vu16 *) 0x0400001a)

#define BG3_XOFFSET ((vu16 *) 0x0400001c)
#define BG3_YOFFSET ((vu16 *) 0x0400001e)

struct Background {
    // Behavior
    u32 priority : 2;
    u32 overflow : 1; // (only BG2/BG3) 0 = transparent, 1 = wraparound

    // Size
    u32 size : 2;

    // Tileset and Tilemap
    u32 tileset : 2; // units of 16K
    u32 tilemap : 5; // units of 2K

    // Other
    u32 mosaic     : 1;
    u32 color_mode : 1; // 0 = 16/16 palettes, 1 = 256/1 palette
};

ALWAYS_INLINE
inline void background_config(const struct Background *config, u32 id) {
    if(id >= BACKGROUND_COUNT)
        return;

    vu16 *control_registers[BACKGROUND_COUNT] = {
        BG0_CONTROL, BG1_CONTROL, BG2_CONTROL, BG3_CONTROL
    };

    *control_registers[id] = config->priority   << 0  |
                             config->tileset    << 2  |
                             config->mosaic     << 6  |
                             config->color_mode << 7  |
                             config->tilemap    << 8  |
                             config->overflow   << 13 |
                             config->size       << 14;
}

ALWAYS_INLINE
inline void background_set_offset(u16 x, u16 y, u32 id) {
    if(id >= BACKGROUND_COUNT)
        return;

    struct {
        vu16 *x;
        vu16 *y;
    } offset_registers[BACKGROUND_COUNT] = {
        { BG0_XOFFSET, BG0_YOFFSET },
        { BG1_XOFFSET, BG1_YOFFSET },
        { BG2_XOFFSET, BG2_YOFFSET },
        { BG3_XOFFSET, BG3_YOFFSET }
    };

    *(offset_registers[id].x) = x;
    *(offset_registers[id].y) = y;
}
