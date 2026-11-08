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
#include "libsimplegba/cart/backup.h"

#include "libsimplegba/hw/dma.h"

#define EEPROM ((vu8 *) 0x0dffff00)

SBSS_SECTION
static u16 eeprom_buffer[67 + DRIVER_ADDRLEN];

static INLINE void encode_request(u32 address, bool write_flag) {
    // read or write request
    eeprom_buffer[0] = 1;
    eeprom_buffer[1] = write_flag;

    // encode address (most significant bit first)
    for(u32 i = 0; i < DRIVER_ADDRLEN; i++)
        eeprom_buffer[2 + i] = address >> (DRIVER_ADDRLEN - 1 - i);

    // terminator bit
    eeprom_buffer[2 + DRIVER_ADDRLEN + (write_flag * 64)] = 0;
}

THUMB
static void read_block(u32 address, u8 block[8]) {
    encode_request(address, false);

    dma_config(DMA3, &(struct DMA) {
        .dest_control = DMA_ADDR_INCREMENT,
        .src_control  = DMA_ADDR_INCREMENT,
        .chunk        = DMA_CHUNK_16_BIT,
        .start_timing = DMA_START_IMMEDIATELY,
        .repeat       = false
    });

    // TODO disable interrupts
    dma_transfer(DMA3, EEPROM, eeprom_buffer, 3 + DRIVER_ADDRLEN);
    dma_transfer(DMA3, eeprom_buffer, EEPROM, 68);

    for(u32 i = 4; i < 68; i++) {
        // TODO
    }
}

THUMB
static void write_block(u32 address, const u8 block[8]) {
    encode_request(address, true);

    // encode block data (most significant bit first)
    for(u32 i = 0; i < 64; i++) {
        const u32 byte  = block[7 - i / 8];
        const u32 shift = 7 - i % 8;

        eeprom_buffer[2 + DRIVER_ADDRLEN + i] = byte >> shift;
    }

    dma_config(DMA3, &(struct DMA) {
        .dest_control = DMA_ADDR_INCREMENT,
        .src_control  = DMA_ADDR_INCREMENT,
        .chunk        = DMA_CHUNK_16_BIT,
        .start_timing = DMA_START_IMMEDIATELY,
        .repeat       = false
    });

    // TODO disable interrupts
    dma_transfer(DMA3, EEPROM, eeprom_buffer, 67 + DRIVER_ADDRLEN);
}

THUMB
static void eeprom_read(u16 offset, void *buffer, u32 n) {
    // TODO
}

THUMB
static void eeprom_write(u16 offset, const void *buffer, u32 n) {
    // TODO
}

THUMB
static void eeprom_erase_sector(i32 n) {
    const u8 block[8] = {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff
    };

    #if DRIVER_ADDRLEN == 6
        const u32 start = 0;
        const u32 end   = 64;
    #else
        const u32 start = (n & 1) * 512;
        const u32 end   = start + 512;
    #endif

    for(u32 i = start; i < end; i++)
        write_block(i, block);
}

THUMB
static void eeprom_erase_chip(void) {
    eeprom_erase_sector(0);

    #if DRIVER_ADDRLEN == 14
        eeprom_erase_sector(1);
    #endif
}

THUMB
static u16 eeprom_identify(void) {
    // nothing to do
    return 0;
}

THUMB
static void eeprom_bank(i32 bank) {
    // nothing to do
}

const struct BackupDriver DRIVER_NAME = {
    .id = "EEPROM_Vnnn",

    .read  = eeprom_read,
    .write = eeprom_write,

    .erase_chip   = eeprom_erase_chip,
    .erase_sector = eeprom_erase_sector,

    .identify = eeprom_identify,
    .bank     = eeprom_bank
};
