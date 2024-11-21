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

#define BACKGROUND_COUNT 4

#define BG0 0
#define BG1 1
#define BG2 2
#define BG3 3

// Color mode
#define BACKGROUND_COLORS_16  0
#define BACKGROUND_COLORS_256 1

struct Background {
    u16 priority : 2; // 0-3, 0=highest
    u16 tileset  : 2; // 0-3, in units of 16 KB
    u16 mosaic   : 1; // 0=disable, 1=enable
    u16 colors   : 1; // 0=16-color mode, 1=256-color mode
    u16 tilemap  : 5; // 0-31, in units of 2 KB
    u16 overflow : 1; // 0=transparent, 1=wraparound (only BG2/BG3)
    u16 size     : 2; // 0-3
};

#define _BACKGROUND_GET_CONTROL(id) ((vu16 *) (0x04000008 + id * 2))
#define _BACKGROUND_GET_OFFSET(id)  ((vu32 *) (0x04000010 + id * 4))

INLINE void background_config(u32 id, const struct Background *config) {
    if(id >= BACKGROUND_COUNT)
        return;

    vu16 *control = _BACKGROUND_GET_CONTROL(id);
    *control = config->priority << 0  |
               config->tileset  << 2  |
               config->mosaic   << 6  |
               config->colors   << 7  |
               config->tilemap  << 8  |
               config->overflow << 13 |
               config->size     << 14;
}

INLINE void background_offset(u32 id, u16 x, u16 y) {
    if(id >= BACKGROUND_COUNT)
        return;

    vu32 *offset = _BACKGROUND_GET_OFFSET(id);
    *offset = (x | y << 16);
}

INLINE void background_toggle(u32 id, bool enable) {
    if(id >= BACKGROUND_COUNT)
        return;

    const u16 bit = BIT(8 + id);

    vu16 *display_control = (vu16 *) 0x04000000;
    if(enable)
        *display_control |= bit;
    else
        *display_control &= ~bit;
}

// set the mosaic values of backgrounds
INLINE void background_mosaic(u32 x, u32 y) {
    vu8 *mosaic = (vu8 *) 0x0400004c;
    *mosaic = (x & 15) | (y & 15) << 4;
}

#undef _BACKGROUND_GET_CONTROL
#undef _BACKGROUND_GET_OFFSET
