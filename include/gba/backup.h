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
#pragma once

#include <base.h>

extern const struct _BackupDriver {
    void (*read) (u16 offset, void *buffer, u32 n);
    void (*write)(u16 offset, const void *buffer, u32 n);

    u8   (*read_byte) (u16 offset);
    void (*write_byte)(u16 offset, u8 byte);

    u16  (*identify)(void);
    void (*set_bank)(u32 bank);

    void (*erase_chip)  (void);
    void (*erase_sector)(u32 n);
} *_backup_driver;

INLINE void backup_init(const void *driver) {
    _backup_driver = (const struct _BackupDriver *) driver;
}

INLINE void backup_read(u16 offset, void *buffer, u32 n) {
    _backup_driver->read(offset, buffer, n);
}

INLINE void backup_write(u16 offset, const void *buffer, u32 n) {
    _backup_driver->write(offset, buffer, n);
}

INLINE u8 backup_read_byte(u16 offset) {
    return _backup_driver->read_byte(offset);
}

INLINE void backup_write_byte(u16 offset, u8 byte) {
    _backup_driver->write_byte(offset, byte);
}

INLINE u16 backup_identify(void) {
    if(_backup_driver->identify)
        return _backup_driver->identify();
    return 0;
}

INLINE void backup_set_bank(u32 bank) {
    if(_backup_driver->set_bank)
        _backup_driver->set_bank(bank);
}

INLINE void backup_erase_chip(void) {
    if(_backup_driver->erase_chip)
        _backup_driver->erase_chip();
}

INLINE void backup_erase_sector(u32 n) {
    if(_backup_driver->erase_sector)
        _backup_driver->erase_sector(n);
}

extern const struct _BackupDriver
    _backup_driver_sram,
    _backup_driver_flash,
    _backup_driver_eeprom;

#define BACKUP_SRAM   (&_backup_driver_sram)
#define BACKUP_FLASH  (&_backup_driver_flash)
#define BACKUP_EEPROM (&_backup_driver_eeprom)
