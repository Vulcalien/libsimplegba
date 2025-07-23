/* Copyright 2023-2024 Vulcalien
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

#define INTERRUPT_COUNT 14

#define IRQ_VBLANK  0
#define IRQ_HBLANK  1
#define IRQ_VCOUNT  2

#define IRQ_TIMER0  3
#define IRQ_TIMER1  4
#define IRQ_TIMER2  5
#define IRQ_TIMER3  6

#define IRQ_SERIAL  7

#define IRQ_DMA0    8
#define IRQ_DMA1    9
#define IRQ_DMA2    10
#define IRQ_DMA3    11

#define IRQ_KEYPAD  12
#define IRQ_GAMEPAK 13

INLINE void interrupt_toggle(i32 id, bool enable) {
    if(id < 0 || id >= INTERRUPT_COUNT)
        return;

    const struct {
        u16 offset;
        u16 bit;
    } senders[INTERRUPT_COUNT] = {
        [IRQ_VBLANK]  = { 0x0004, BIT(3)  },
        [IRQ_HBLANK]  = { 0x0004, BIT(4)  },
        [IRQ_VCOUNT]  = { 0x0004, BIT(5)  },
        [IRQ_TIMER0]  = { 0x0102, BIT(6)  },
        [IRQ_TIMER1]  = { 0x0106, BIT(6)  },
        [IRQ_TIMER2]  = { 0x010a, BIT(6)  },
        [IRQ_TIMER3]  = { 0x010e, BIT(6)  },
        [IRQ_SERIAL]  = { 0x0128, BIT(14) },
        [IRQ_DMA0]    = { 0x00ba, BIT(14) },
        [IRQ_DMA1]    = { 0x00c6, BIT(14) },
        [IRQ_DMA2]    = { 0x00d2, BIT(14) },
        [IRQ_DMA3]    = { 0x00de, BIT(14) },
        [IRQ_KEYPAD]  = { 0x0132, BIT(14) },
        [IRQ_GAMEPAK] = { 0,      0       }, // no register
    };

    vu16 *sender = (vu16 *) (0x04000000 + senders[id].offset);
    vu16 *ie = (vu16 *) 0x04000200;

    // toggle IRQ bits in both sender register and IE
    if(enable) {
        *sender |= senders[id].bit;
        *ie     |= BIT(id);
    } else {
        *sender &= ~senders[id].bit;
        *ie     &= ~BIT(id);
    }
}

extern void interrupt_set_isr(i32 id, void (*isr)(void));
extern void interrupt_wait(i32 id);
