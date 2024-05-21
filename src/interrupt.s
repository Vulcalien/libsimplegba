@ Copyright 2024 Vulcalien
@
@ This program is free software: you can redistribute it and/or modify
@ it under the terms of the GNU General Public License as published by
@ the Free Software Foundation, either version 3 of the License, or
@ (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program.  If not, see <https://www.gnu.org/licenses/>.

.include "macros.inc"

.equ IME,               0x04000208  @ 32-bit
.equ IE,                0x04000200  @ 16-bit
.equ IF,                0x04000202  @ 16-bit
.equ IF_BIOS,           0x03007ff8  @ 16-bit

.equ INTERRUPT_HANDLER, 0x03007ffc  @ 32-bit

.equ INTERRUPT_COUNT, 14

@ --- isr_table (32-bit array) --- @
.bss
.align 2

@ a table of function pointers

isr_table:
    .space (INTERRUPT_COUNT * 4)

@ --- interrupt_handler --- @
.section .iwram, "ax"
ARM_FUNC

@ Handle IRQs by calling the matching ISR.
@ Tasks of this function:
@   - read IF and determine which IRQ was raised
@   - acknowledge that IRQ (both in IF and IF_BIOS)
@   - check if that IRQ has a matching ISR
@       - if not, return
@       - if ISR exists continue
@   - switch to System Mode
@   - call the ISR
@   - switch to IRQ Mode
@
@ It should be uncommon for multiple IRQs to be raised at the same time,
@ so the function only serves one interrupt and then exits. In the event
@ that other bits were to be set, the BIOS would simply call this
@ function again.

interrupt_handler:
    @ load IF
    ldr     r0, =IF                     @ r0 = pointer to IF
    ldrh    r1, [r0]                    @ r1 = IF value

    @ find the IRQ bit
    mov     r2, #0                      @ r2 = loop counter (0)
1: @ loop
    @ if loop counter >= INTERRUPT_COUNT, return
    cmp     r2, #(INTERRUPT_COUNT)
    bxge    lr

    @ calculate (1 << loop counter)
    mov     r3, #1
    lsl     r3, r2                      @ r3 = (1 << loop counter)

    @ check if the IRQ bit in IF is set
    tst     r1, r3                      @ test IF & (1 << irq)

    @ if the bit is clear, continue
    addeq   r2, #1                      @ loop counter += 1
    beq     1b @ loop

    @ --- the IRQ bit was found --- @

    @ acknowledge the IRQ
    strh    r3, [r0]                    @ IF = (1 << loop counter)

    ldr     r0, =IF_BIOS                @ r0 = pointer to IF_BIOS
    ldrh    r1, [r0]                    @ r1 = IF_BIOS value
    orr     r1, r3
    strh    r1, [r0]                    @ IF_BIOS |= (1 << loop counter)

    @ get the ISR
    ldr     r0, =isr_table              @ r0 = pointer to isr_table
    ldr     r0, [r0, r2, lsl #2]        @ r0 = isr_table[loop counter]

    @ if the ISR is NULL, return
    cmp     r0, #0
    bxeq    lr

    @ switch to System mode
    mov     r1, #0x9f
    msr     cpsr, r1

    @ call the ISR
    push    {lr}
    ldr     lr, =2f
    bx      r0
2: @ ISR return
    pop     {lr}

    @ switch back to IRQ mode
    mov     r1, #0x92
    msr     cpsr, r1

    bx      lr

.align
.pool

@ --- interrupt_init --- @
.global interrupt_init
.text
THUMB_FUNC

interrupt_init:
    @ set interrupt vector
    ldr     r0, =INTERRUPT_HANDLER
    ldr     r1, =interrupt_handler
    str     r1, [r0]

    @ IME = 1
    ldr     r0, =IME
    mov     r1, #1
    str     r1, [r0]

    bx      lr

.align
.pool

@ --- interrupt_set_isr --- @
.global interrupt_set_isr
.text
ARM_FUNC

@ r0 = irq
@ r1 = isr
interrupt_set_isr:
    @ check if the given IRQ is valid
    cmp     r0, #(INTERRUPT_COUNT)

    @ isr_table[irq] = irq
    ldrlo   r2, =isr_table
    strlo   r1, [r2, r0, lsl #2]

    bx      lr

.end
