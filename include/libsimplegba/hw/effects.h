/* Copyright 2026 Vulcalien
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

struct EffectsTarget {
    u8 bg0 : 1;
    u8 bg1 : 1;
    u8 bg2 : 1;
    u8 bg3 : 1;

    u8 obj : 1;
    u8 backdrop : 1;
};

#define _EFFECTS_CONTROL    *((vu16 *) 0x04000050)
#define _EFFECTS_ALPHA      *((vu16 *) 0x04000052)
#define _EFFECTS_BRIGHTNESS *((vu16 *) 0x04000054)

#define _EFFECTS_DEFAULT_TARGET (&(struct EffectsTarget) {          \
    .bg0 = 1, .bg1 = 1, .bg2 = 1, .bg3 = 1, .obj = 1, .backdrop = 1 \
})

INLINE void effects_blend(const struct EffectsTarget *first,
                          const struct EffectsTarget *second,
                          i32 weight_1, i32 weight_2) {
    first  = (first  ? first  : _EFFECTS_DEFAULT_TARGET);
    second = (second ? second : _EFFECTS_DEFAULT_TARGET);

    _EFFECTS_CONTROL = first->bg0      << 0  |
                       first->bg1      << 1  |
                       first->bg2      << 2  |
                       first->bg3      << 3  |
                       first->obj      << 4  |
                       first->backdrop << 5  |
                       1                    << 6  |
                       second->bg0      << 8  |
                       second->bg1      << 9  |
                       second->bg2      << 10 |
                       second->bg3      << 11 |
                       second->obj      << 12 |
                       second->backdrop << 13;

    _EFFECTS_ALPHA = (weight_1 & 0x1f) | (weight_2 & 0x1f) << 8;
}

INLINE void effects_brighten(const struct EffectsTarget *target,
                             i32 weight) {
    target = (target ? target : _EFFECTS_DEFAULT_TARGET);

    _EFFECTS_CONTROL = target->bg0      << 0 |
                       target->bg1      << 1 |
                       target->bg2      << 2 |
                       target->bg3      << 3 |
                       target->obj      << 4 |
                       target->backdrop << 5 |
                       2                << 6;
    _EFFECTS_BRIGHTNESS = weight & 0x1f;
}

INLINE void effects_darken(const struct EffectsTarget *target,
                           i32 weight) {
    target = (target ? target : _EFFECTS_DEFAULT_TARGET);

    _EFFECTS_CONTROL = target->bg0      << 0 |
                       target->bg1      << 1 |
                       target->bg2      << 2 |
                       target->bg3      << 3 |
                       target->obj      << 4 |
                       target->backdrop << 5 |
                       3                << 6;
    _EFFECTS_BRIGHTNESS = weight & 0x1f;
}

INLINE void effects_disable(void) {
    _EFFECTS_CONTROL = 0;
}

#undef _EFFECTS_CONTROL
#undef _EFFECTS_ALPHA
#undef _EFFECTS_BRIGHTNESS

#undef _EFFECTS_DEFAULT_TARGET
