/* Copyright 2023-2024 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <base.h>

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

#define _INTERRUPT_IE *((vu16 *) 0x04000200)

// toggle the IRQ sender bit of the corresponding register
INLINE void _interrupt_toggle_register_bit(u8 irq, bool enable) {
    if(irq >= INTERRUPT_COUNT)
        return;

    const struct {
        u16 addr_offset;
        u16 bit;
    } registers[INTERRUPT_COUNT] = {
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
        [IRQ_GAMEPAK] = { 0,      0       }, // (no register)
    };

    vu16 *address = (vu16 *) (0x04000000 + registers[irq].addr_offset);
    u16 bit = registers[irq].bit;

    // check if the given IRQ has no sender bit (only IRQ_GAMEPAK)
    if(!bit)
        return;

    if(enable)
        *address |= bit;
    else
        *address &= ~bit;
}

extern void interrupt_init(void);

INLINE void interrupt_enable(u8 irq) {
    _interrupt_toggle_register_bit(irq, true);
    _INTERRUPT_IE |= (1 << irq);
}

INLINE void interrupt_disable(u8 irq) {
    _INTERRUPT_IE &= ~(1 << irq);
    _interrupt_toggle_register_bit(irq, false);
}

extern void interrupt_set_isr(u8 irq, void (*isr)(void));

#undef _INTERRUPT_IE
