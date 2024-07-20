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
#include <gba/backup.h>

#define BACKUP_ADDR ((vu8 *) 0x0e000000)

THUMB
u16 backup_flash_identify(void) {
    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0x90;

    u16 result = BACKUP_ADDR[0x0000] | BACKUP_ADDR[0x0001] << 8;

    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0xf0;

    return result;
}

THUMB
void backup_flash_set_bank(u32 bank) {
    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0xb0;

    BACKUP_ADDR[0x0000] = bank & 1;
}

THUMB
void backup_flash_write(u16 address, u8 byte) {
    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0xa0;

    BACKUP_ADDR[address] = byte;

    while(BACKUP_ADDR[address] != byte);
}

THUMB
void backup_flash_erase_chip(void) {
    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0x80;

    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0x10;

    while(BACKUP_ADDR[0x0000] != 0xff);
}

THUMB
void backup_flash_erase_sector(u32 n) {
    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[0x5555] = 0x80;

    BACKUP_ADDR[0x5555] = 0xaa;
    BACKUP_ADDR[0x2aaa] = 0x55;
    BACKUP_ADDR[n << 12] = 0x30;

    while(BACKUP_ADDR[n << 12] != 0xff);
}
