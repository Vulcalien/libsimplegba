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
#include "libsimplegba/math.h"

#define SCALE 128

static const u16 atan_lut[0x4000 / SCALE + 1] = {
    0x0000, 0x0051, 0x00a2, 0x00f4, 0x0145, 0x0197, 0x01e8, 0x0239,
    0x028b, 0x02dc, 0x032d, 0x037e, 0x03ce, 0x041f, 0x0470, 0x04c0,
    0x0511, 0x0561, 0x05b1, 0x0601, 0x0650, 0x06a0, 0x06ef, 0x073e,
    0x078d, 0x07db, 0x082a, 0x0878, 0x08c6, 0x0913, 0x0961, 0x09ae,
    0x09fb, 0x0a47, 0x0a94, 0x0ae0, 0x0b2b, 0x0b77, 0x0bc2, 0x0c0c,
    0x0c57, 0x0ca1, 0x0ceb, 0x0d34, 0x0d7d, 0x0dc6, 0x0e0e, 0x0e56,
    0x0e9e, 0x0ee5, 0x0f2c, 0x0f72, 0x0fb8, 0x0ffe, 0x1044, 0x1089,
    0x10cd, 0x1111, 0x1155, 0x1199, 0x11dc, 0x121e, 0x1260, 0x12a2,
    0x12e4, 0x1325, 0x1365, 0x13a5, 0x13e5, 0x1424, 0x1463, 0x14a2,
    0x14e0, 0x151e, 0x155b, 0x1598, 0x15d4, 0x1610, 0x164c, 0x1687,
    0x16c2, 0x16fc, 0x1736, 0x1770, 0x17a9, 0x17e2, 0x181a, 0x1852,
    0x188a, 0x18c1, 0x18f7, 0x192e, 0x1964, 0x1999, 0x19ce, 0x1a03,
    0x1a37, 0x1a6b, 0x1a9f, 0x1ad2, 0x1b05, 0x1b37, 0x1b69, 0x1b9b,
    0x1bcc, 0x1bfd, 0x1c2e, 0x1c5e, 0x1c8e, 0x1cbd, 0x1cec, 0x1d1b,
    0x1d49, 0x1d77, 0x1da5, 0x1dd2, 0x1dff, 0x1e2c, 0x1e58, 0x1e84,
    0x1eaf, 0x1eda, 0x1f05, 0x1f30, 0x1f5a, 0x1f84, 0x1fad, 0x1fd7,
    0x2000
};

INLINE i32 octantify(i32 *x, i32 *y) {
    i32 phi = 0;
    if(*y < 0) {
        const i32 x0 = *x;
        const i32 y0 = *y;

        *x = -x0;
        *y = -y0;
        phi -= math_brad(180);
    }

    if(*x <= 0) {
        const i32 x0 = *x;
        const i32 y0 = *y;

        *x = y0;
        *y = -x0;
        phi += math_brad(90);
    }

    if(*x < *y) {
        const i32 x0 = *x;
        const i32 y0 = *y;

        *x = x0 + y0;
        *y = y0 - x0;
        phi += math_brad(45);
    }
    return phi;
}

INLINE u32 safe_ratio(u32 num, u32 den, u16 scale) {
    // If (num * scale) would overflow, divide both parts by scale.
    //
    // Note that, in such case, (den / scale) is always nonzero:
    // 1. den >= num   |  because of octantify
    // 2. num > scale  |  since (scale < 2^16) and (num * scale >= 2^32)
    if(num > U32_MAX / scale) {
        num /= scale;
        den /= scale;
    }
    return num * scale / den;
}

THUMB
i32 math_atan2(i32 y, i32 x) {
    if(x == 0 && y == 0)
        return 0;

    i32 phi = octantify(&x, &y);
    u32 ratio = safe_ratio(y, x, 0x4000);

    u32 index  = ratio / SCALE;
    u32 weight = ratio % SCALE;

    u16 left  = atan_lut[index];
    u16 right = atan_lut[index + 1];

    i32 result = left + (right - left) * weight / SCALE;
    return phi + result;
}
