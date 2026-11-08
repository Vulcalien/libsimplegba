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
#include "libsimplegba/hw/serial.h"

#include "libsimplegba/hw/interrupt.h"

#define SERIAL_CONTROL *((vu16 *) 0x04000128)
#define SERIAL_MODE    *((vu16 *) 0x04000134)

#define SERIAL_DATA  ((vu32 *) 0x04000120)
#define SERIAL_SEND *((vu16 *) 0x0400012a)

// control bits
#define MASTER_BIT BIT(2)
#define READY_BIT  BIT(3)
#define ACTIVE_BIT BIT(7)
#define IRQ_BIT    BIT(14)

#define MULTIPLAYER_MODE BIT(13)

static volatile bool transfer_complete = true;

static void serial_isr(void) {
    transfer_complete = true;
}

static void delay(void) {
    for(vi32 i = 0; i < 1000; i++)
        ;
}

static i32 wait_for_ready(void) {
    for(i32 i = 0; true; i++) {
        if(SERIAL_CONTROL & READY_BIT)
            return 0;
    }
    return 1;
}

static i32 wait_transfer_complete(void) {
    for(i32 i = 0; true; i++) {
        if(transfer_complete)
            return 0;
    }
    return 1;
}

i32 multiplayer_transfer(const void *outbuf, void *inbuf, u32 n) {
    interrupt_toggle(IRQ_SERIAL, true);
    interrupt_isr(IRQ_SERIAL, serial_isr);

    SERIAL_MODE = 0;
    SERIAL_CONTROL = (SERIAL_CONTROL & IRQ_BIT) | MULTIPLAYER_MODE;

    if(wait_for_ready())
        return 1;

    const bool is_master = !(SERIAL_CONTROL & MASTER_BIT);
    for(i32 i = 0; i < n / 2; i++) {
        transfer_complete = false;

        // prepare outgoing 16-bit data
        if(outbuf)
            SERIAL_SEND = ((u16 *) outbuf)[i];

        if(is_master) {
            SERIAL_CONTROL |= ACTIVE_BIT;
            delay();
        }

        if(wait_transfer_complete())
            return 2;

        // process incoming 64-bit data
        if(inbuf) {
            ((u32 *) inbuf)[i * 2 + 0] = SERIAL_DATA[0];
            ((u32 *) inbuf)[i * 2 + 1] = SERIAL_DATA[1];
        }
    }
    return 0;
}

const struct SerialDriver _serial_driver_multiplayer = {
    .transfer = multiplayer_transfer
};
