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
#include "debug/mgba.h"

#define MGBA_DEBUG_ENABLE *((vu16 *) 0x04fff780)
#define MGBA_DEBUG_FLAGS  *((vu16 *) 0x04fff700)
#define MGBA_DEBUG_STRING  ((char *) 0x04fff600)

static u8 debug_level;

bool mgba_open(void) {
    mgba_log_level(MGBA_LOG_INFO); // default log level is 'info'

    MGBA_DEBUG_ENABLE = 0xc0de;
    return MGBA_DEBUG_ENABLE == 0x1dea;
}

void mgba_close(void) {
    MGBA_DEBUG_ENABLE = 0;
}

void mgba_log_level(u32 level) {
    debug_level = level;
}

void mgba_puts(const char *str) {
    for(u32 i = 0; i < 256; i++) {
        MGBA_DEBUG_STRING[i] = str[i];

        // After '\0' was written, break.
        // If the string's length is greater than or equal to 256 then
        // '\0' is never written and the string is trimmed, but mGBA
        // treats it as if it had a '\0' at the end.
        if(str[i] == '\0')
            break;
    }
    MGBA_DEBUG_FLAGS = debug_level | 0x100;
}
