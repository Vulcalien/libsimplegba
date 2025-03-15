/* Copyright 2025 Vulcalien
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
#include <math.h>

#define SCALE 32

// lookup table for values between 0 and 0x4000
static const u16 asin_lut[0x4000 / SCALE + 2] = {
    0x0000, 0x0014, 0x0028, 0x003d, 0x0051, 0x0065, 0x007a, 0x008e,
    0x00a2, 0x00b7, 0x00cb, 0x00e0, 0x00f4, 0x0108, 0x011d, 0x0131,
    0x0146, 0x015a, 0x016e, 0x0183, 0x0197, 0x01ab, 0x01c0, 0x01d4,
    0x01e9, 0x01fd, 0x0211, 0x0226, 0x023a, 0x024f, 0x0263, 0x0277,
    0x028c, 0x02a0, 0x02b5, 0x02c9, 0x02dd, 0x02f2, 0x0306, 0x031b,
    0x032f, 0x0344, 0x0358, 0x036d, 0x0381, 0x0395, 0x03aa, 0x03be,
    0x03d3, 0x03e7, 0x03fc, 0x0410, 0x0425, 0x0439, 0x044e, 0x0462,
    0x0477, 0x048b, 0x04a0, 0x04b4, 0x04c9, 0x04dd, 0x04f2, 0x0506,
    0x051b, 0x052f, 0x0544, 0x0558, 0x056d, 0x0581, 0x0596, 0x05ab,
    0x05bf, 0x05d4, 0x05e8, 0x05fd, 0x0612, 0x0626, 0x063b, 0x064f,
    0x0664, 0x0679, 0x068d, 0x06a2, 0x06b7, 0x06cb, 0x06e0, 0x06f4,
    0x0709, 0x071e, 0x0733, 0x0747, 0x075c, 0x0771, 0x0785, 0x079a,
    0x07af, 0x07c4, 0x07d8, 0x07ed, 0x0802, 0x0817, 0x082b, 0x0840,
    0x0855, 0x086a, 0x087f, 0x0893, 0x08a8, 0x08bd, 0x08d2, 0x08e7,
    0x08fc, 0x0911, 0x0926, 0x093a, 0x094f, 0x0964, 0x0979, 0x098e,
    0x09a3, 0x09b8, 0x09cd, 0x09e2, 0x09f7, 0x0a0c, 0x0a21, 0x0a36,
    0x0a4b, 0x0a60, 0x0a75, 0x0a8a, 0x0a9f, 0x0ab4, 0x0ac9, 0x0adf,
    0x0af4, 0x0b09, 0x0b1e, 0x0b33, 0x0b48, 0x0b5e, 0x0b73, 0x0b88,
    0x0b9d, 0x0bb2, 0x0bc8, 0x0bdd, 0x0bf2, 0x0c07, 0x0c1d, 0x0c32,
    0x0c47, 0x0c5d, 0x0c72, 0x0c87, 0x0c9d, 0x0cb2, 0x0cc8, 0x0cdd,
    0x0cf3, 0x0d08, 0x0d1d, 0x0d33, 0x0d48, 0x0d5e, 0x0d73, 0x0d89,
    0x0d9f, 0x0db4, 0x0dca, 0x0ddf, 0x0df5, 0x0e0b, 0x0e20, 0x0e36,
    0x0e4c, 0x0e61, 0x0e77, 0x0e8d, 0x0ea3, 0x0eb8, 0x0ece, 0x0ee4,
    0x0efa, 0x0f10, 0x0f25, 0x0f3b, 0x0f51, 0x0f67, 0x0f7d, 0x0f93,
    0x0fa9, 0x0fbf, 0x0fd5, 0x0feb, 0x1001, 0x1017, 0x102d, 0x1043,
    0x1059, 0x106f, 0x1086, 0x109c, 0x10b2, 0x10c8, 0x10de, 0x10f5,
    0x110b, 0x1121, 0x1138, 0x114e, 0x1164, 0x117b, 0x1191, 0x11a8,
    0x11be, 0x11d5, 0x11eb, 0x1202, 0x1218, 0x122f, 0x1245, 0x125c,
    0x1273, 0x1289, 0x12a0, 0x12b7, 0x12cd, 0x12e4, 0x12fb, 0x1312,
    0x1329, 0x133f, 0x1356, 0x136d, 0x1384, 0x139b, 0x13b2, 0x13c9,
    0x13e0, 0x13f7, 0x140e, 0x1426, 0x143d, 0x1454, 0x146b, 0x1482,
    0x149a, 0x14b1, 0x14c8, 0x14e0, 0x14f7, 0x150e, 0x1526, 0x153d,
    0x1555, 0x156c, 0x1584, 0x159c, 0x15b3, 0x15cb, 0x15e3, 0x15fa,
    0x1612, 0x162a, 0x1642, 0x1659, 0x1671, 0x1689, 0x16a1, 0x16b9,
    0x16d1, 0x16e9, 0x1701, 0x171a, 0x1732, 0x174a, 0x1762, 0x177a,
    0x1793, 0x17ab, 0x17c4, 0x17dc, 0x17f4, 0x180d, 0x1825, 0x183e,
    0x1857, 0x186f, 0x1888, 0x18a1, 0x18ba, 0x18d2, 0x18eb, 0x1904,
    0x191d, 0x1936, 0x194f, 0x1968, 0x1981, 0x199a, 0x19b4, 0x19cd,
    0x19e6, 0x1a00, 0x1a19, 0x1a32, 0x1a4c, 0x1a65, 0x1a7f, 0x1a99,
    0x1ab2, 0x1acc, 0x1ae6, 0x1b00, 0x1b19, 0x1b33, 0x1b4d, 0x1b67,
    0x1b81, 0x1b9c, 0x1bb6, 0x1bd0, 0x1bea, 0x1c05, 0x1c1f, 0x1c39,
    0x1c54, 0x1c6e, 0x1c89, 0x1ca4, 0x1cbe, 0x1cd9, 0x1cf4, 0x1d0f,
    0x1d2a, 0x1d45, 0x1d60, 0x1d7b, 0x1d96, 0x1db2, 0x1dcd, 0x1de8,
    0x1e04, 0x1e1f, 0x1e3b, 0x1e57, 0x1e73, 0x1e8e, 0x1eaa, 0x1ec6,
    0x1ee2, 0x1efe, 0x1f1a, 0x1f37, 0x1f53, 0x1f6f, 0x1f8c, 0x1fa8,
    0x1fc5, 0x1fe2, 0x1ffe, 0x201b, 0x2038, 0x2055, 0x2072, 0x208f,
    0x20ad, 0x20ca, 0x20e7, 0x2105, 0x2123, 0x2140, 0x215e, 0x217c,
    0x219a, 0x21b8, 0x21d6, 0x21f4, 0x2213, 0x2231, 0x2250, 0x226e,
    0x228d, 0x22ac, 0x22cb, 0x22ea, 0x2309, 0x2328, 0x2348, 0x2367,
    0x2387, 0x23a7, 0x23c6, 0x23e6, 0x2407, 0x2427, 0x2447, 0x2467,
    0x2488, 0x24a9, 0x24ca, 0x24eb, 0x250c, 0x252d, 0x254e, 0x2570,
    0x2591, 0x25b3, 0x25d5, 0x25f7, 0x2619, 0x263c, 0x265e, 0x2681,
    0x26a4, 0x26c7, 0x26ea, 0x270e, 0x2731, 0x2755, 0x2779, 0x279d,
    0x27c1, 0x27e5, 0x280a, 0x282f, 0x2854, 0x2879, 0x289f, 0x28c4,
    0x28ea, 0x2910, 0x2936, 0x295d, 0x2984, 0x29ab, 0x29d2, 0x29f9,
    0x2a21, 0x2a49, 0x2a71, 0x2a9a, 0x2ac2, 0x2aec, 0x2b15, 0x2b3e,
    0x2b68, 0x2b93, 0x2bbd, 0x2be8, 0x2c13, 0x2c3f, 0x2c6b, 0x2c97,
    0x2cc4, 0x2cf1, 0x2d1e, 0x2d4c, 0x2d7a, 0x2da8, 0x2dd8, 0x2e07,
    0x2e37, 0x2e67, 0x2e98, 0x2eca, 0x2efc, 0x2f2e, 0x2f61, 0x2f95,
    0x2fc9, 0x2ffe, 0x3034, 0x306a, 0x30a1, 0x30d9, 0x3111, 0x314a,
    0x3184, 0x31bf, 0x31fb, 0x3238, 0x3276, 0x32b5, 0x32f5, 0x3337,
    0x3379, 0x33bd, 0x3403, 0x344a, 0x3493, 0x34dd, 0x352a, 0x3578,
    0x35c9, 0x361d, 0x3673, 0x36cc, 0x3729, 0x378a, 0x37ef, 0x3859,
    0x38c9, 0x3941, 0x39c1, 0x3a4d, 0x3ae7, 0x3b96, 0x3c65, 0x3d73,
    0x4000, 0 // this number is here to prevent access out of bounds
};

// LUT-based implementation of arcsine. Since asin(-x) = -asin(x), the
// table only contains entries for non-negative arguments.

i32 math_asin(i32 x) {
    u32 abs_val = math_abs(x);

    // if argument is outside the domain, return 0
    if(abs_val > 0x4000)
        return 0;

    u32 index  = abs_val / SCALE;
    u32 weight = abs_val % SCALE;

    u16 left  = asin_lut[index];
    u16 right = asin_lut[index + 1];

    // interpolate LUT entries
    i32 result = (left * (SCALE - weight) + right * weight) / SCALE;

    // adjust sign
    if(x < 0)
        result *= -1;

    return result;
}
