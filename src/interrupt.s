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
@   - read IE & IF and determine which IRQ was raised
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
    @ load (IE & IF)
    ldr     r0, =0x04000200             @ r0 = pointer to IE_IF
    ldr     r1, [r0]                    @ r1 = IE_IF
    and     r1, r1, lsr #16             @ r1 = IE & IF

    @ find the IRQ bit in (IE & IF)
    mov     r2, #0                      @ r2 = IRQ id  (0)
    mov     r3, #1                      @ r3 = IRQ bit (1)
1: @ loop
    @ if IRQ id >= INTERRUPT_COUNT, return
    cmp     r2, #(INTERRUPT_COUNT)
    bxge    lr

    @ check if the IRQ bit in (IE & IF) is set
    tst     r1, r3                      @ test (IE & IF) & (IRQ bit)

    @ if the bit is clear, continue
    addeq   r2, #1                      @ IRQ id += 1
    lsleq   r3, #1                      @ IRQ bit <<= 1
    beq     1b @ loop

    @ --- the IRQ bit was found --- @

    @ acknowledge the IRQ
    strh    r3, [r0, #0x2]              @ IF = IRQ bit

    ldr     r0, =0x03007ff8             @ r0 = pointer to IF_BIOS
    ldrh    r1, [r0]                    @ r1 = IF_BIOS
    orr     r1, r3                      @ r1 = IF_BIOS | IRQ bit
    strh    r1, [r0]                    @ IF_BIOS = (IF_BIOS | IRQ bit)

    @ get the ISR
    ldr     r0, =isr_table              @ r0 = pointer to isr_table
    ldr     r0, [r0, r2, lsl #2]        @ r0 = isr_table[IRQ id]

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

.size interrupt_handler, .-interrupt_handler

.align
.pool

@ --- _interrupt_init --- @
.global _interrupt_init
.text
THUMB_FUNC

_interrupt_init:
    @ set interrupt vector
    ldr     r0, =0x03007ffc             @ r0 = pointer to master ISR
    ldr     r1, =interrupt_handler
    str     r1, [r0]

    @ IME = 1
    ldr     r0, =0x04000208             @ r0 = pointer to IME
    mov     r1, #1
    str     r1, [r0]

    bx      lr

.size _interrupt_init, .-_interrupt_init

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

.size interrupt_set_isr, .-interrupt_set_isr

.end
