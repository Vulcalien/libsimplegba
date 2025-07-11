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
#include "libsimplegba/hw/backup.h"

#include "libsimplegba/memory.h"

#define SRAM ((vu8 *) 0x0e000000)

THUMB
static void sram_read(u16 offset, void *buffer, u32 n) {
    memory_copy_8(buffer, SRAM + offset, n);
}

THUMB
static void sram_write(u16 offset, const void *buffer, u32 n) {
    memory_copy_8(SRAM + offset, buffer, n);
}

THUMB
static i32 sram_read_byte(u16 offset) {
    return SRAM[offset];
}

THUMB
static void sram_write_byte(u16 offset, i32 byte) {
    SRAM[offset] = byte;
}

const struct _BackupDriver _backup_driver_sram = {
    .read  = sram_read,
    .write = sram_write,

    .read_byte  = sram_read_byte,
    .write_byte = sram_write_byte
};
