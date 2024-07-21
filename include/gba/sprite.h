/* Copyright 2023-2024 Vulcalien
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

#define OAM ((vu16 *) 0x07000000)

// Sprite sizes:
//   | Size | Square | Horizontal | Vertical |
//   | ---- | ------ | ---------- | -------- |
//   |   0  |   8x8  |    16x8    |   8x16   |
//   |   1  |  16x16 |    32x8    |   8x32   |
//   |   2  |  32x32 |    32x16   |   16x32  |
//   |   3  |  64x64 |    64x32   |   32x64  |

struct Sprite {
    u32 x : 9;
    u32 y : 8;

    u32 disable  : 1; // 0=false, 1=true
    u32 mode     : 2; // 0=normal, 1=semi-transparent, 2=window
    u32 priority : 2;

    u32 shape : 2; // 0=square, 1=horizontal, 2=vertical
    u32 size  : 2; // see 'Sprite size'
    u32 flip  : 2; // 0=none, 1=horizontal, 2=vertical, 3=both

    u32 tile    : 10;
    u32 palette : 4;

    u32 mosaic : 1; // 0=disable, 1=enable
    u32 colors : 1; // 0=16 palettes of 16, 1=1 palette of 256

    u32 affine           : 1; // 0=disable, 1=enable
    u32 affine_parameter : 5;
    u32 double_size      : 1; // 0=disable, 1=enable
};

INLINE void sprite_config(u32 id, const struct Sprite *sprite) {
    if(id >= SPRITE_COUNT)
        return;

    vu16 *attribs = &OAM[id * 4];

    u32 attr0_bit_9;
    u32 attr1_bits_9_13;

    if(sprite->affine) {
        attr0_bit_9     = sprite->double_size;
        attr1_bits_9_13 = sprite->affine_parameter;
    } else {
        attr0_bit_9     = sprite->disable;
        attr1_bits_9_13 = sprite->flip << 3;
    }

    attribs[0] = sprite->y      << 0  |
                 sprite->affine << 8  |
                 attr0_bit_9    << 9  |
                 sprite->mode   << 10 |
                 sprite->mosaic << 12 |
                 sprite->colors << 13 |
                 sprite->shape  << 14;

    attribs[1] = sprite->x       << 0  |
                 attr1_bits_9_13 << 9  |
                 sprite->size    << 14;

    attribs[2] = sprite->tile     << 0  |
                 sprite->priority << 10 |
                 sprite->palette  << 12;
}

INLINE void sprite_hide(u32 id) {
    if(id >= SPRITE_COUNT)
        return;

    vu16 *attribs = &OAM[id * 4];
    attribs[0] = (1 << 9);
}

// 'start' is included, 'stop' is not included
INLINE void sprite_hide_range(u32 start, u32 stop) {
    for(u32 i = start; i < stop; i++)
        sprite_hide(i);
}

INLINE void sprite_hide_all(void) {
    sprite_hide_range(0, SPRITE_COUNT);
}

// set the mosaic values of sprites
INLINE void sprite_mosaic(u32 x, u32 y) {
    vu8 *mosaic = (vu8 *) 0x0400004d;
    *mosaic = (x & 15) | (y & 15) << 4;
}
