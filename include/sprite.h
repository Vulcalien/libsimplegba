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

#include "base.h"

#define SPRITE_COUNT (128)

#define OAM ((vu16 *) 0x07000000)

struct Sprite {
    // Position
    u32 x : 9;
    u32 y : 8;

    // Behavior
    u32 disable  : 1;
    u32 mode     : 2;
    u32 priority : 2;

    // Shape, Size and Flip
    u32 shape : 2; // 0 = square, 1 = horizontal, 2 = vertical
    u32 size  : 2;
    u32 flip  : 2;

    // Tile and Palette
    u32 tile    : 10;
    u32 palette : 4;

    // Affine Transformation
    u32 affine_transformation : 1;
    u32 affine_parameter      : 5;
    u32 double_size           : 1;

    // Other
    u32 mosaic     : 1;
    u32 color_mode : 1; // 0 = 16/16 palettes, 1 = 256/1 palette
};

INLINE void sprite_config(u32 id, const struct Sprite *sprite) {
    if(id >= SPRITE_COUNT)
        return;

    vu16 *attribs = &OAM[id * 4];

    u32 attr0_bit_9;
    u32 attr1_bits_9_13;

    if(sprite->affine_transformation) {
        attr0_bit_9     = sprite->double_size;
        attr1_bits_9_13 = sprite->affine_parameter;
    } else {
        attr0_bit_9     = sprite->disable;
        attr1_bits_9_13 = sprite->flip << 3;
    }

    attribs[0] = sprite->y                     << 0  |
                 sprite->affine_transformation << 8  |
                 attr0_bit_9                   << 9  |
                 sprite->mode                  << 10 |
                 sprite->mosaic                << 12 |
                 sprite->color_mode            << 13 |
                 sprite->shape                 << 14;

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
