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

#define DISPLAY_BG_PALETTE  ((vu16 *) 0x05000000)
#define DISPLAY_OBJ_PALETTE ((vu16 *) 0x05000200)

#define _DISPLAY_CONTROL *((vu16 *) 0x04000000)
#define _DISPLAY_STATUS  *((vu16 *) 0x04000004)
#define _DISPLAY_VCOUNT  *((vu16 *) 0x04000006)

// Video Modes
//
// Tilemap modes:
//   | Mode | BG0 | BG1 | BG2 | BG3 |
//   | ---- | --- | --- | --- | --- |
//   |   0  | reg | reg | reg | reg |
//   |   1  | reg | reg | aff |     |
//   |   2  |     |     | aff | aff |
// reg: regular background
// aff: affine background
//
// Bitmap modes:
//   | Mode |   Size  | BPP | Pages |
//   | ---- | ------- | --- | ----- |
//   |   3  | 240x160 |  16 |   1   |
//   |   4  | 240x160 |  8  |   2   |
//   |   5  | 160x128 |  16 |   2   |

struct Display {
    u16 mode : 3; // see 'Video Modes'

    u16 oam_hblank  : 1; // 1=allow access to OAM during H-Blank
    u16 obj_mapping : 1; // 0=32x32 matrix, 1=linear

    u16 enable_bg0 : 1; // 0=false, 1=true
    u16 enable_bg1 : 1; // 0=false, 1=true
    u16 enable_bg2 : 1; // 0=false, 1=true
    u16 enable_bg3 : 1; // 0=false, 1=true
    u16 enable_obj : 1; // 0=false, 1=true
};

INLINE void display_config(const struct Display *config) {
    // clear bits used by this function
    u16 val = _DISPLAY_CONTROL & ~(0x1f67);

    if(config) {
        val |= config->mode        << 0  |
               config->oam_hblank  << 5  |
               config->obj_mapping << 6  |
               config->enable_bg0  << 8  |
               config->enable_bg1  << 9  |
               config->enable_bg2  << 10 |
               config->enable_bg3  << 11 |
               config->enable_obj  << 12;
    }
    _DISPLAY_CONTROL = val;
}

INLINE void display_force_blank(bool flag) {
    if(flag)
        _DISPLAY_CONTROL |= BIT(7);
    else
        _DISPLAY_CONTROL &= ~BIT(7);
}

INLINE u16 display_get_vcount(void) {
    return _DISPLAY_VCOUNT;
}

// Charblocks are 16 KB areas of VRAM. There are 6 charblocks. They are
// used to store tilesets.
//
// Note that charblocks 0-3 and screenblocks share the same memory area.
//
// charblocks 0-3: background tiles (only in Tilemap modes)
// charblocks 4-5: sprite tiles (only charblock 5 in Bitmap modes)
INLINE vu16 *display_get_charblock(u32 block) {
    return (vu16 *) (0x06000000 + block * 0x4000);
}

// Screenblocks are 2 KB areas of VRAM. There are 32 screenblocks. They
// are used to store background tilemaps.
//
// Note that charblocks 0-3 and screenblocks share the same memory area.
INLINE vu16 *display_get_screenblock(u32 block) {
    return (vu16 *) (0x06000000 + block * 0x800);
}

// === Bitmap-specific ===

INLINE u32 display_get_page(void) {
    return (_DISPLAY_CONTROL >> 4) & 1;
}

INLINE void display_set_page(u32 page) {
    if(page & 1)
        _DISPLAY_CONTROL |= BIT(4);
    else
        _DISPLAY_CONTROL &= ~BIT(4);
}

INLINE vu16 *display_get_raster(u32 page) {
    return (vu16 *) (0x06000000 + (page & 1) * 0xa000);
}

#undef _DISPLAY_CONTROL
#undef _DISPLAY_STATUS
#undef _DISPLAY_VCOUNT
