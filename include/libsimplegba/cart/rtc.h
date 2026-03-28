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

struct RTC {
    u8 year;  // [0, 99]
    u8 month; // [1, 12]
    u8 day;   // [1, 31]

    u8 hour;   // [0, 23]
    u8 minute; // [0, 59]
    u8 second; // [0, 59]

    u8 day_of_week; // [0, 6]
};

extern void rtc_read(struct RTC *datetime);
extern void rtc_write(const struct RTC *datetime);
