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
#include "libsimplegba/cart/backup.h"

#include "libsimplegba/memory.h"

#define FLASH ((vu8 *) 0x0e000000)

THUMB
static i32 flash_read_byte(u16 offset) {
    return FLASH[offset];
}

THUMB
static void flash_write_byte(u16 offset, i32 byte) {
    byte &= 0xff;

    // prepare to write byte
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0xa0;

    FLASH[offset] = byte;

    while(FLASH[offset] != byte);
}

THUMB
static void flash_read(u16 offset, void *buffer, u32 n) {
    memory_copy_8(buffer, FLASH + offset, n);
}

THUMB
static void flash_write(u16 offset, const void *buffer, u32 n) {
    for(u32 i = 0; i < n; i++)
        flash_write_byte(offset + i, ((u8 *) buffer)[i]);
}

THUMB
static u16 flash_identify(void) {
    // enter chip identification mode
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0x90;

    u16 result = FLASH[0x0000] | FLASH[0x0001] << 8;

    // exit chip identification mode
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0xf0;

    return result;
}

THUMB
static void flash_set_bank(u32 bank) {
    // prepare to set memory bank
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0xb0;

    FLASH[0x0000] = (bank & 1);
}

THUMB
static void flash_erase_chip(void) {
    // prepare to erase
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0x80;

    // erase entire chip
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0x10;

    while(FLASH[0x0000] != 0xff);
}

THUMB
static void flash_erase_sector(u32 n) {
    n %= 16;

    // prepare to erase
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[0x5555] = 0x80;

    // erase n-th sector
    FLASH[0x5555] = 0xaa;
    FLASH[0x2aaa] = 0x55;
    FLASH[n << 12] = 0x30;

    while(FLASH[n << 12] != 0xff);
}

const struct _BackupDriver _backup_driver_flash = {
    .read  = flash_read,
    .write = flash_write,

    .read_byte  = flash_read_byte,
    .write_byte = flash_write_byte,

    .identify = flash_identify,
    .set_bank = flash_set_bank,

    .erase_chip   = flash_erase_chip,
    .erase_sector = flash_erase_sector
};
