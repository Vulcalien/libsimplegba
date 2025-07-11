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

#define _DISPLAY_CONTROL *((vu16 *) 0x04000000)

INLINE void display_config(u32 video_mode) {
    if(video_mode >= 6)
        return;

    // clear all bits except force blank
    u16 val = _DISPLAY_CONTROL & BIT(7);

    val |= video_mode;
    val |= 0 << 4  | // select raster page 0
           0 << 5  | // disable access to OAM during HBlank
           1 << 6  | // linear sprite mapping
           0 << 8  | // disable all backgrounds
           1 << 12 | // enable sprites
           0 << 13;  // disable all windows

    // if configuring a bitmap Video Mode, enable BG 2
    if(video_mode >= 3)
        val |= BIT(10);

    _DISPLAY_CONTROL = val;
}

INLINE void display_force_blank(bool flag) {
    if(flag)
        _DISPLAY_CONTROL |= BIT(7);
    else
        _DISPLAY_CONTROL &= ~BIT(7);
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

INLINE i32 display_get_page(void) {
    return (_DISPLAY_CONTROL >> 4) & 1;
}

INLINE void display_set_page(i32 page) {
    if(page & 1)
        _DISPLAY_CONTROL |= BIT(4);
    else
        _DISPLAY_CONTROL &= ~BIT(4);
}

INLINE vu16 *display_get_raster(i32 page) {
    return (vu16 *) (0x06000000 + (page & 1) * 0xa000);
}

// === Graphic Effects ===

struct DisplayTarget {
    u8 bg0 : 1;
    u8 bg1 : 1;
    u8 bg2 : 1;
    u8 bg3 : 1;

    u8 obj : 1;

    u8 backdrop : 1;
};

#define _DISPLAY_EFFECTS_CONTROL    *((vu16 *) 0x04000050)
#define _DISPLAY_EFFECTS_ALPHA      *((vu16 *) 0x04000052)
#define _DISPLAY_EFFECTS_BRIGHTNESS *((vu16 *) 0x04000054)

#define _DISPLAY_DEFAULT_TARGET (&(struct DisplayTarget) {          \
    .bg0 = 1, .bg1 = 1, .bg2 = 1, .bg3 = 1, .obj = 1, .backdrop = 1 \
})

INLINE void display_blend(const struct DisplayTarget *target_1st,
                          const struct DisplayTarget *target_2nd,
                          u32 weight_1st, u32 weight_2nd) {
    target_1st = (target_1st ? target_1st : _DISPLAY_DEFAULT_TARGET);
    target_2nd = (target_2nd ? target_2nd : _DISPLAY_DEFAULT_TARGET);

    _DISPLAY_EFFECTS_CONTROL = target_1st->bg0      << 0  |
                               target_1st->bg1      << 1  |
                               target_1st->bg2      << 2  |
                               target_1st->bg3      << 3  |
                               target_1st->obj      << 4  |
                               target_1st->backdrop << 5  |
                               1                    << 6  |
                               target_2nd->bg0      << 8  |
                               target_2nd->bg1      << 9  |
                               target_2nd->bg2      << 10 |
                               target_2nd->bg3      << 11 |
                               target_2nd->obj      << 12 |
                               target_2nd->backdrop << 13;

    _DISPLAY_EFFECTS_ALPHA = (weight_1st & 0x1f) << 0 |
                             (weight_2nd & 0x1f) << 8;
}

INLINE void display_brighten(const struct DisplayTarget *target,
                             u32 weight) {
    target = (target ? target : _DISPLAY_DEFAULT_TARGET);

    _DISPLAY_EFFECTS_CONTROL = target->bg0      << 0 |
                               target->bg1      << 1 |
                               target->bg2      << 2 |
                               target->bg3      << 3 |
                               target->obj      << 4 |
                               target->backdrop << 5 |
                               2                << 6;
    _DISPLAY_EFFECTS_BRIGHTNESS = weight & 0x1f;
}

INLINE void display_darken(const struct DisplayTarget *target,
                           u32 weight) {
    target = (target ? target : _DISPLAY_DEFAULT_TARGET);

    _DISPLAY_EFFECTS_CONTROL = target->bg0      << 0 |
                               target->bg1      << 1 |
                               target->bg2      << 2 |
                               target->bg3      << 3 |
                               target->obj      << 4 |
                               target->backdrop << 5 |
                               3                << 6;
    _DISPLAY_EFFECTS_BRIGHTNESS = weight & 0x1f;
}

INLINE void display_disable_effects(void) {
    _DISPLAY_EFFECTS_CONTROL = 0;
}

#undef _DISPLAY_EFFECTS_CONTROL
#undef _DISPLAY_EFFECTS_ALPHA
#undef _DISPLAY_EFFECTS_BRIGHTNESS

#undef _DISPLAY_DEFAULT_TARGET

// ===== ===== =====

#undef _DISPLAY_CONTROL
