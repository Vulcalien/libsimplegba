/* Copyright 2024, 2026 Vulcalien
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

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 160

#define DISPLAY_BG_PALETTE  ((vu16 *) 0x05000000)
#define DISPLAY_OBJ_PALETTE ((vu16 *) 0x05000200)

// Tilemap video modes
#define DISPLAY_MODE_0 0
#define DISPLAY_MODE_1 1
#define DISPLAY_MODE_2 2

// Bitmap video modes
#define DISPLAY_MODE_3 3
#define DISPLAY_MODE_4 4
#define DISPLAY_MODE_5 5

#define _DISPLAY_CONTROL *((vu16 *) 0x04000000)

INLINE void display_config(i32 mode) {
    // side effects:
    // - set raster page to 0
    // - disable all backgrounds (except BG2 if bitmap mode)
    // - disable all windows
    _DISPLAY_CONTROL = (
        (mode & 7) << 0  | // video mode
        1          << 6  | // linear sprite mapping
        (mode < 0) << 7  | // force blank if mode < 0
        (mode > 2) << 10 | // enable BG2 if bitmap mode
        1          << 12   // enable sprites
    );
}

// TODO This function remains undocumented: should it really exist?
// What about setting the VCount in the DISPLAY_STATUS register?
// Or reading the VBlank/HBlank/VCount flags in DISPLAY_STATUS?
INLINE u16 display_vcount(void) {
    return *(vu16 *) 0x04000006;
}

// Charblocks 0-5 are 16 KB areas of VRAM used to store tilesets.
//
// Tilemap modes: 0-3 background tilesets, 4-5 sprite tileset.
// Bitmap modes: 0-4 raster, 5 sprite tileset.
INLINE vu16 *display_charblock(i32 block) {
    return (vu16 *) (0x06000000 + block * 0x4000);
}

// Screenblocks 0-31 are 2 KB areas of VRAM used to store tilemaps.
INLINE vu16 *display_screenblock(i32 block) {
    return (vu16 *) (0x06000000 + block * 0x800);
}

// === Bitmap-specific ===

INLINE vu16 *display_raster(i32 page) {
    return (vu16 *) (0x06000000 + (page & 1) * 0xa000);
}

INLINE i32 display_get_page(void) {
    return (_DISPLAY_CONTROL >> 4) & 1;
}

INLINE void display_set_page(i32 page) {
    if(page & 1)
        _DISPLAY_CONTROL |= BIT(4);
    else
        _DISPLAY_CONTROL &= ~BIT(4);
}

#undef _DISPLAY_CONTROL
