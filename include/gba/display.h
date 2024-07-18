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

    u16 oam_hblank  : 1; // 1=allow access to OAM during HBlank
    u16 obj_mapping : 1; // 0=32x32 matrix, 1=linear

    u16 enable_bg0 : 1; // 0=false, 1=true
    u16 enable_bg1 : 1; // 0=false, 1=true
    u16 enable_bg2 : 1; // 0=false, 1=true
    u16 enable_bg3 : 1; // 0=false, 1=true
    u16 enable_obj : 1; // 0=false, 1=true
};

#define _DISPLAY_CONTROL *((vu16 *) 0x04000000)
#define _DISPLAY_STATUS  *((vu16 *) 0x04000004)
#define _DISPLAY_VCOUNT  *((vu16 *) 0x04000006)

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

INLINE void display_effects_disable(void) {
    _DISPLAY_EFFECTS_CONTROL = 0;
}

#undef _DISPLAY_EFFECTS_CONTROL
#undef _DISPLAY_EFFECTS_ALPHA
#undef _DISPLAY_EFFECTS_BRIGHTNESS

#undef _DISPLAY_DEFAULT_TARGET

// === Windows ===

#define DISPLAY_WINDOW_COUNT 4

#define DISPLAY_WINDOW_0   0
#define DISPLAY_WINDOW_1   1
#define DISPLAY_WINDOW_OUT 2
#define DISPLAY_WINDOW_OBJ 3

struct DisplayWindow {
    u8 bg0 : 1;
    u8 bg1 : 1;
    u8 bg2 : 1;
    u8 bg3 : 1;

    u8 obj : 1;

    u8 effects : 1;
};

INLINE void display_window_config(u32 id,
                                  struct DisplayWindow *config) {
    if(id > DISPLAY_WINDOW_COUNT)
        return;

    // if config is NULL, use a default configuration instead
    config = (config ? config : (&(struct DisplayWindow) {
        .bg0 = 1, .bg1 = 1, .bg2 = 1, .bg3 = 1, .obj = 1, .effects = 1
    }));

    vu32 *control = (vu32 *) 0x04000048;

    // clear the window's bits
    u32 val = *control & ~(0x3f << id * 8);

    // set the window's bit as configured
    val |= (
        config->bg0     << 0 |
        config->bg1     << 1 |
        config->bg2     << 2 |
        config->bg3     << 3 |
        config->obj     << 4 |
        config->effects << 5
    ) << (id * 8);
    *control = val;
}

INLINE void display_window_viewport(u32 id,
                                    u32 x, u32 y,
                                    u32 width, u32 height) {
    if(id != DISPLAY_WINDOW_0 && id != DISPLAY_WINDOW_1)
        return;

    vu16 *horizontal = (vu16 *) (0x04000040 + id * 2);
    vu16 *vertical   = (vu16 *) (0x04000044 + id * 2);

    *horizontal = ((x & 0xff) << 8 | ((x + width) & 0xff));
    *vertical   = ((y & 0xff) << 8 | ((y + height) & 0xff));
}

INLINE void _display_window_toggle_enable_bit(u32 id, bool enable) {
    if(id > DISPLAY_WINDOW_COUNT)
        return;

    u16 bit = (u16 [DISPLAY_WINDOW_COUNT]) {
        [DISPLAY_WINDOW_0]   = BIT(13),
        [DISPLAY_WINDOW_1]   = BIT(14),
        [DISPLAY_WINDOW_OUT] = 0,
        [DISPLAY_WINDOW_OBJ] = BIT(15)
    }[id];

    if(enable)
        _DISPLAY_CONTROL |= bit;
    else
        _DISPLAY_CONTROL &= ~bit;
}

INLINE void display_window_enable(u32 id) {
    _display_window_toggle_enable_bit(id, true);
}

INLINE void display_window_disable(u32 id) {
    _display_window_toggle_enable_bit(id, false);
}

// ===== ===== =====

#undef _DISPLAY_CONTROL
#undef _DISPLAY_STATUS
#undef _DISPLAY_VCOUNT
