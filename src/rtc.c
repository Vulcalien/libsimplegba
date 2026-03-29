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
#include "libsimplegba/cart/rtc.h"

#include "libsimplegba/cart/gpio.h"

#define CMD_READ_DATETIME  0xa6
#define CMD_WRITE_DATETIME 0x26

static const u8 bin_to_bcd[100] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99
};

static INLINE i32 bcd(i32 bin) {
    return bin_to_bcd[bin];
}

THUMB
static NO_INLINE i32 bin(i32 bcd) {
    return (bcd & 15) + (bcd >> 4) * 10;
}

THUMB
static NO_INLINE void delay(void) {
    #pragma GCC unroll 0
    for(vi32 i = 0; i < 8; i++);
}

THUMB
static NO_INLINE i32 read_byte(void) {
    gpio_config(5); // CS=out, SIO=in, SCK=out

    i32 result = 0;
    for(i32 i = 0; i < 8; i++) {
        gpio_write(4); // CS=1, SCK=0
        delay();
        gpio_write(5); // CS=1, SCK=1
        delay();

        i32 x = (gpio_read() >> 1) & 1;
        result |= (x << i);
    }
    return result;
}

THUMB
static NO_INLINE void write_byte(i32 val) {
    gpio_config(7); // CS=out, SIO=out, SCK=out

    for(i32 i = 0; i < 8; i++) {
        i32 x = (val >> i) & 1;

        gpio_write(4 | x << 1); // CS=1, SIO=x, SCK=0
        delay();
        gpio_write(5 | x << 1); // CS=1, SIO=x, SCK=1
        delay();
    }
}

THUMB
void rtc_read(struct RTC *datetime) {
    gpio_toggle(true);

    gpio_config(7); // CS=out, SIO=out, SCK=out
    gpio_write(1); // CS=0, SIO=0, SCK=1
    delay();
    gpio_write(5); // CS=1, SIO=0, SCK=1
    delay();

    write_byte(CMD_READ_DATETIME);

    datetime->year        = bin(read_byte());
    datetime->month       = bin(read_byte());
    datetime->day         = bin(read_byte());
    datetime->day_of_week = bin(read_byte());
    datetime->hour        = bin(read_byte());
    datetime->minute      = bin(read_byte());
    datetime->second      = bin(read_byte());

    gpio_write(1); // CS=0, SIO=0, SCK=1
}

THUMB
void rtc_write(const struct RTC *datetime) {
    gpio_toggle(true);

    gpio_config(7); // CS=out, SIO=out, SCK=out
    gpio_write(1); // CS=0, SIO=0, SCK=1
    delay();
    gpio_write(5); // CS=1, SIO=0, SCK=1
    delay();

    write_byte(CMD_WRITE_DATETIME);

    write_byte(bcd(datetime->year));
    write_byte(bcd(datetime->month));
    write_byte(bcd(datetime->day));
    write_byte(bcd(datetime->day_of_week));
    write_byte(bcd(datetime->hour));
    write_byte(bcd(datetime->minute));
    write_byte(bcd(datetime->second));

    gpio_write(1); // CS=0, SIO=0, SCK=1
}
