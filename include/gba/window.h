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

#define WINDOW_COUNT 4

#define WINDOW_0   0
#define WINDOW_1   1
#define WINDOW_OUT 2
#define WINDOW_OBJ 3

struct Window {
    u8 bg0 : 1;
    u8 bg1 : 1;
    u8 bg2 : 1;
    u8 bg3 : 1;

    u8 obj : 1;

    u8 effects : 1;
};

INLINE void window_config(u32 id, struct Window *config) {
    if(id >= WINDOW_COUNT)
        return;

    // if config is NULL, use a default configuration instead
    config = (config ? config : (&(struct Window) {
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

INLINE void window_viewport(u32 id, u32 x, u32 y,
                            u32 width, u32 height) {
    if(id != WINDOW_0 && id != WINDOW_1)
        return;

    vu16 *horizontal = (vu16 *) (0x04000040 + id * 2);
    vu16 *vertical   = (vu16 *) (0x04000044 + id * 2);

    *horizontal = ((x & 0xff) << 8 | ((x + width) & 0xff));
    *vertical   = ((y & 0xff) << 8 | ((y + height) & 0xff));
}

INLINE void _window_toggle_enable_bit(u32 id, bool enable) {
    if(id >= WINDOW_COUNT)
        return;

    u16 bit = (u16 [WINDOW_COUNT]) {
        [WINDOW_0]   = BIT(13),
        [WINDOW_1]   = BIT(14),
        [WINDOW_OUT] = 0,
        [WINDOW_OBJ] = BIT(15)
    }[id];

    vu16 *display_control = (vu16 *) 0x04000000;
    if(enable)
        *display_control |= bit;
    else
        *display_control &= ~bit;
}

INLINE void window_enable(u32 id) {
    _window_toggle_enable_bit(id, true);
}

INLINE void window_disable(u32 id) {
    _window_toggle_enable_bit(id, false);
}
