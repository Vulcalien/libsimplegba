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
#include "test.h"

static i32 fails  = 0;
static i32 passes = 0;

void test_run(bool (*test)(void), const char *name) {
    bool result = test();
    if(result) {
        passes++;
        mgba_printf("%s passed", name);
    } else {
        fails++;
        mgba_printf("%s failed", name);
    }
}

void AgbMain(void) {
    test_run_all();

    mgba_printf("Passed: %d / %d", passes, fails + passes);
    while(true) {}
}
