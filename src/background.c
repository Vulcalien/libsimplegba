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
#include "background.h"

#define BG0_CONTROL ((vu16 *) 0x04000008)
#define BG1_CONTROL ((vu16 *) 0x0400000a)
#define BG2_CONTROL ((vu16 *) 0x0400000c)
#define BG3_CONTROL ((vu16 *) 0x0400000e)

#define BG0_XOFFSET ((vu16 *) 0x04000010)
#define BG0_YOFFSET ((vu16 *) 0x04000012)

#define BG1_XOFFSET ((vu16 *) 0x04000014)
#define BG1_YOFFSET ((vu16 *) 0x04000016)

#define BG2_XOFFSET ((vu16 *) 0x04000018)
#define BG2_YOFFSET ((vu16 *) 0x0400001a)

#define BG3_XOFFSET ((vu16 *) 0x0400001c)
#define BG3_YOFFSET ((vu16 *) 0x0400001e)

const struct Background backgrounds[4] = {
    // BG0
    {
        .control = BG0_CONTROL,
        .offset = { BG0_XOFFSET, BG0_YOFFSET }
    },

    // BG1
    {
        .control = BG1_CONTROL,
        .offset = { BG1_XOFFSET, BG1_YOFFSET }
    },

    // BG2
    {
        .control = BG2_CONTROL,
        .offset = { BG2_XOFFSET, BG2_YOFFSET }
    },

    // BG3
    {
        .control = BG3_CONTROL,
        .offset = { BG3_XOFFSET, BG3_YOFFSET }
    }
};
