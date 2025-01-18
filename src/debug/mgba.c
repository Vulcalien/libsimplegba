/* Copyright 2024-2025 Vulcalien
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
#include <debug/mgba.h>

#include <stdarg.h>

#include <math.h>

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

static INLINE void print_unsigned(u32 val, u32 base, u32 *index) {
    // count digits
    u32 len = 0;
    {
        u32 tmp = val;
        do {
            len++;
            tmp /= base;
        } while(tmp != 0);
    }

    *index += len;
    for(u32 i = 0; i < len; i++) {
        u32 digit = val % base;
        val /= base;

        digit += (digit < 10 ? '0' : 'a' - 10);
        MGBA_DEBUG_STRING[*index - i - 1] = digit;
    }
}

// This function does not implement the full functionality of 'printf',
// but only a very small subset:
//   %% - '%' character
//   %d - i32 (signed decimal)
//   %u - u32 (unsigned decimal)
//   %x - u32 (hexadecimal)
//   %c - char
void mgba_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    bool converting_special = false;
    for(u32 index = 0; index < 256; format++) {
        char c = *format;

        if(converting_special) {
            if(c == '%') {
                MGBA_DEBUG_STRING[index++] = '%';
            } else if(c == 'c') {
                u32 val = va_arg(args, u32);
                MGBA_DEBUG_STRING[index++] = (char) val;
            } else if(c == 'd') {
                i32 val = va_arg(args, i32);

                // if negative, write sign
                if(val < 0)
                    MGBA_DEBUG_STRING[index++] = '-';

                // get absolute value of number
                u32 abs;
                if(val != I32_MIN)
                    abs = math_abs(val);
                else
                    abs = (u32) I32_MAX + 1;

                print_unsigned(abs, 10, &index);
            } else if(c == 'u') {
                u32 val = va_arg(args, u32);
                print_unsigned(val, 10, &index);
            } else if(c == 'x') {
                u32 val = va_arg(args, u32);
                print_unsigned(val, 16, &index);
            }

            converting_special = false;
        } else {
            if(c == '%')
                converting_special = true;
            else
                MGBA_DEBUG_STRING[index++] = c; // might be '\0'
        }

        if(c == '\0')
            break;
    }
    va_end(args);

    MGBA_DEBUG_FLAGS = debug_level | 0x100;
}
