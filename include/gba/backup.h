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

#include <base.h>

#include <memory.h>

#define _BACKUP_ADDR ((vu8 *) 0x0e000000)

// === SRAM ===

INLINE u8 backup_sram_read_byte(u16 address) {
    return _BACKUP_ADDR[address];
}

INLINE void backup_sram_read(u16 offset, void *buffer, u32 n) {
    memory_copy_8(buffer, _BACKUP_ADDR + offset, n);
}

INLINE void backup_sram_write_byte(u16 address, u8 byte) {
    _BACKUP_ADDR[address] = byte;
}

INLINE void backup_sram_write(u16 offset, const void *buffer, u32 n) {
    memory_copy_8(_BACKUP_ADDR + offset, buffer, n);
}

// === Flash ROM ===

extern u16 backup_flash_identify(void);
extern void backup_flash_set_bank(u32 bank);

INLINE u8 backup_flash_read_byte(u16 address) {
    return _BACKUP_ADDR[address];
}

INLINE void backup_flash_read(u16 offset, void *buffer, u32 n) {
    memory_copy_8(buffer, _BACKUP_ADDR + offset, n);
}

extern void backup_flash_write_byte(u16 address, u8 byte);

INLINE void backup_flash_write(u16 offset, const void *buffer, u32 n) {
    for(u32 i = 0; i < n; i++)
        backup_flash_write_byte(offset + i, ((u8 *) buffer)[i]);
}

extern void backup_flash_erase_chip(void);
extern void backup_flash_erase_sector(u32 n);

// === EEPROM ===

// TODO

// ===== ===== =====

#undef _BACKUP_ADDR
