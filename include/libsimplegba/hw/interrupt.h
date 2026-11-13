/* Copyright 2023-2024, 2026 Vulcalien
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

#define _INTERRUPT_COUNT 14

INLINE void interrupt_config(i32 id, i32 config) {
    switch(id) {
        case IRQ_VCOUNT: {
            vu8 *vcount = (vu8 *) 0x04000005;
            *vcount = config;
        } break;

        case IRQ_KEYPAD: {
            vu16 *control = (vu16 *) 0x04000132;
            const u16 irq = BIT(14);

            u16 tmp = *control & irq;
            tmp |= config & ~irq;
            *control = tmp;
        } break;
    }
}

extern void interrupt_handler(i32 id, void (*isr)(void));

INLINE void interrupt_mask(i32 id, bool masked) {
    if(id < 0 || id >= _INTERRUPT_COUNT)
        return;

    vu16 *ie = (vu16 *) 0x04000200;
    if(masked)
        *ie &= ~BIT(id);
    else
        *ie |= BIT(id);
}

INLINE void interrupt_toggle(i32 id, bool enable) {
    if(id < 0 || id >= _INTERRUPT_COUNT)
        return;

    const struct {
        u32 address;
        u16 bit;
    } senders[_INTERRUPT_COUNT] = {
        [IRQ_VBLANK]  = { 0x04000004, BIT(3)  },
        [IRQ_HBLANK]  = { 0x04000004, BIT(4)  },
        [IRQ_VCOUNT]  = { 0x04000004, BIT(5)  },
        [IRQ_TIMER0]  = { 0x04000102, BIT(6)  },
        [IRQ_TIMER1]  = { 0x04000106, BIT(6)  },
        [IRQ_TIMER2]  = { 0x0400010a, BIT(6)  },
        [IRQ_TIMER3]  = { 0x0400010e, BIT(6)  },
        [IRQ_SERIAL]  = { 0x04000128, BIT(14) },
        [IRQ_DMA0]    = { 0x040000ba, BIT(14) },
        [IRQ_DMA1]    = { 0x040000c6, BIT(14) },
        [IRQ_DMA2]    = { 0x040000d2, BIT(14) },
        [IRQ_DMA3]    = { 0x040000de, BIT(14) },
        [IRQ_KEYPAD]  = { 0x04000132, BIT(14) },
        [IRQ_GAMEPAK] = { 0x04000000, 0       }, // no register
    };

    vu16 *sender = (vu16 *) senders[id].address;
    if(enable) {
        *sender |= senders[id].bit;
    } else {
        *sender &= ~senders[id].bit;

        // clear pending requests in IF
        vu16 *if_reg = (vu16 *) 0x04000202;
        *if_reg = BIT(id);
    }

    interrupt_mask(id, !enable);
}

extern void interrupt_wait(i32 id);

#undef _INTERRUPT_COUNT
