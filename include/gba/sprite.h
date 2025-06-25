/* Copyright 2023-2025 Vulcalien
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

#define SPRITE_COUNT 128

// Mode
#define SPRITE_MODE_NORMAL           0
#define SPRITE_MODE_SEMI_TRANSPARENT 1
#define SPRITE_MODE_WINDOW           2

// Square sizes
#define SPRITE_SIZE_8x8   (0 << 2 | 0)
#define SPRITE_SIZE_16x16 (0 << 2 | 1)
#define SPRITE_SIZE_32x32 (0 << 2 | 2)
#define SPRITE_SIZE_64x64 (0 << 2 | 3)

// Horizontal sizes
#define SPRITE_SIZE_16x8  (1 << 2 | 0)
#define SPRITE_SIZE_32x8  (1 << 2 | 1)
#define SPRITE_SIZE_32x16 (1 << 2 | 2)
#define SPRITE_SIZE_64x32 (1 << 2 | 3)

// Vertical sizes
#define SPRITE_SIZE_8x16  (2 << 2 | 0)
#define SPRITE_SIZE_8x32  (2 << 2 | 1)
#define SPRITE_SIZE_16x32 (2 << 2 | 2)
#define SPRITE_SIZE_32x64 (2 << 2 | 3)

// Flip
#define SPRITE_FLIP_NONE       0
#define SPRITE_FLIP_HORIZONTAL 1
#define SPRITE_FLIP_VERTICAL   2
#define SPRITE_FLIP_BOTH       3

// Color mode
#define SPRITE_COLORS_16  0
#define SPRITE_COLORS_256 1

struct Sprite {
    u32 x : 9;
    u32 y : 8;

    u32 disable  : 1; // 0=false, 1=true
    u32 mode     : 2; // 0=normal, 1=semi-transparent, 2=window
    u32 priority : 2; // 0-3, 0=highest

    u32 size : 4; // one of the SPRITE_SIZE_* constants
    u32 flip : 2; // 0=none, 1=horizontal, 2=vertical, 3=both

    u32 tile    : 10;
    u32 palette : 4;

    u32 mosaic : 1; // 0=disable, 1=enable
    u32 colors : 1; // 0=16-color mode, 1=256-color mode

    u32 affine           : 1; // 0=disable, 1=enable
    u32 affine_parameter : 5; // 0-31
    u32 double_size      : 1; // 0=disable, 1=enable
};

#define _SPRITE_OAM ((vu16 *) 0x07000000)

INLINE void sprite_config(i32 id, const struct Sprite *sprite) {
    if(id < 0 || id >= SPRITE_COUNT)
        return;

    vu16 *attribs = &_SPRITE_OAM[id * 4];

    u32 attr0_bit_9;
    u32 attr1_bits_9_13;

    if(sprite->affine) {
        attr0_bit_9     = sprite->double_size;
        attr1_bits_9_13 = sprite->affine_parameter;
    } else {
        attr0_bit_9     = sprite->disable;
        attr1_bits_9_13 = sprite->flip << 3;
    }

    const u32 shape = (sprite->size >> 2) & 3;
    const u32 size = sprite->size & 3;

    // if in 256-colors mode, double the tile number
    const u32 tile = (sprite->tile << sprite->colors) & BITMASK(10);

    attribs[0] = sprite->y      << 0  |
                 sprite->affine << 8  |
                 attr0_bit_9    << 9  |
                 sprite->mode   << 10 |
                 sprite->mosaic << 12 |
                 sprite->colors << 13 |
                 shape          << 14;

    attribs[1] = sprite->x       << 0  |
                 attr1_bits_9_13 << 9  |
                 size            << 14;

    attribs[2] = tile             << 0  |
                 sprite->priority << 10 |
                 sprite->palette  << 12;
}

// 'start' is included, 'stop' is not included
INLINE void sprite_hide_range(i32 start, i32 stop) {
    if(start < 0)
        start = 0;
    if(stop > SPRITE_COUNT)
        stop = SPRITE_COUNT;

    for(i32 i = start; i < stop; i++)
        _SPRITE_OAM[i * 4] = BIT(9);
}

INLINE void sprite_hide(i32 id) {
    if(id < 0)
        sprite_hide_range(0, SPRITE_COUNT);
    else if(id < SPRITE_COUNT)
        sprite_hide_range(id, id + 1);
}

INLINE void sprite_mosaic(u32 x, u32 y) {
    vu8 *mosaic = (vu8 *) 0x0400004d;
    *mosaic = (x & 15) | (y & 15) << 4;
}

INLINE void sprite_affine(u32 parameter, i16 matrix[4]) {
    if(parameter >= 32)
        return;

    for(u32 i = 0; i < 4; i++)
        _SPRITE_OAM[parameter * 16 + i * 4 + 3] = matrix[i];
}

#undef _SPRITE_OAM
