/* Copyright 2023 Vulcalien
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

#include "base.h"

#define IRQ_VBLANK  (0)
#define IRQ_HBLANK  (1)
#define IRQ_VCOUNT  (2)

#define IRQ_TIMER0  (3)
#define IRQ_TIMER1  (4)
#define IRQ_TIMER2  (5)
#define IRQ_TIMER3  (6)

#define IRQ_SERIAL  (7)

#define IRQ_DMA0    (8)
#define IRQ_DMA1    (9)
#define IRQ_DMA2    (10)
#define IRQ_DMA3    (11)

#define IRQ_KEYPAD  (12)
#define IRQ_GAMEPAK (13)

#define _INTERRUPT_IE *((vu16 *) 0x04000200)

extern void interrupt_init(void);

INLINE void interrupt_enable(u8 irq) {
    _INTERRUPT_IE |= (1 << irq);
}

INLINE void interrupt_disable(u8 irq) {
    _INTERRUPT_IE &= ~(1 << irq);
}

extern void interrupt_set_isr(u8 irq, void (*isr)(void));

#undef _INTERRUPT_IE
