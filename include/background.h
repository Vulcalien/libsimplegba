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

struct Background {
    vu16 *control;

    struct {
        vu16 *x;
        vu16 *y;
    } offset;
};

#define BG0 (&backgrounds[0])
#define BG1 (&backgrounds[1])
#define BG2 (&backgrounds[2])
#define BG3 (&backgrounds[3])
extern const struct Background backgrounds[4];

struct background_Config {
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
inline void background_config(const struct Background *bg,
                              const struct background_Config *config) {
    *(bg->control) = config->priority   << 0  |
                     config->tileset    << 2  |
                     config->mosaic     << 6  |
                     config->color_mode << 7  |
                     config->tilemap    << 8  |
                     config->overflow   << 13 |
                     config->size       << 14;
}

ALWAYS_INLINE
inline void background_set_offset(const struct Background *bg,
                                  u16 x, u16 y) {
    *(bg->offset.x) = x;
    *(bg->offset.y) = y;
}
