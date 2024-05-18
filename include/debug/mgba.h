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

#include "base.h"

// This module makes it possible to use the debug registers of mGBA.

#define MGBA_LOG_FATAL (0)
#define MGBA_LOG_ERROR (1)
#define MGBA_LOG_WARN  (2)
#define MGBA_LOG_INFO  (3)
#define MGBA_LOG_DEBUG (4)

extern bool mgba_open(void);
extern void mgba_close(void);

extern void mgba_log_level(u32 level);

extern void mgba_puts(const char *str);
