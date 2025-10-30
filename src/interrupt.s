@ Copyright 2024 Vulcalien
@
@ This program is free software: you can redistribute it and/or modify
@ it under the terms of the GNU General Public License as published by
@ the Free Software Foundation, either version 3 of the License, or
@ (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program. If not, see <https://www.gnu.org/licenses/>.

.include "assembly.inc"

.equ INTERRUPT_COUNT, 14

@ --- isr_table (32-bit array) --- @
.bss
.align 2

@ a table of function pointers

isr_table:
    .space (INTERRUPT_COUNT * 4)
.size isr_table, .-isr_table

@ --- interrupt_handler --- @

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

BEGIN_FUNC IWRAM ARM interrupt_handler
    mov     r0, #0x04000000             @ (r0) address = register base

    @ load (IE & IF)
    ldr     r1, [r0, #0x200]!           @ *IE_IF, address = IE_IF
    and     r1, r1, r1, lsr #16         @ (r1) IE & IF

    @ find the IRQ bit in (IE & IF)
    mov     r2, #0                      @ (r2) irq_id  = 0
    mov     r3, #1                      @ (r3) irq_bit = 1
1: @ loop
    @ if irq_id >= INTERRUPT_COUNT, return
    cmp     r2, #INTERRUPT_COUNT
    bxge    lr

    @ check if the IRQ bit in (IE & IF) is set
    tst     r1, r3                      @ test (IE & IF) & irq_bit

    @ if the bit is clear, continue
    addeq   r2, #1                      @ (r2) irq_id  +=  1
    lsleq   r3, #1                      @ (r3) irq_bit <<= 1
    beq     1b @ loop

    @ --- the IRQ bit was found --- @

    @ acknowledge the IRQ
    strh    r3, [r0, #2]                @ *IF = irq_bit
    sub     r0, #0x200                  @ address = register base
    ldrh    r1, [r0, #-8]               @ =====================
    orr     r1, r3                      @  *IF_BIOS |= irq_bit
    strh    r1, [r0, #-8]               @ =====================

    @ get the ISR
    ldr     r0, =isr_table              @ isr_table
    ldr     r0, [r0, r2, lsl #2]        @ (r0) isr = isr_table[irq_id]

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
END_FUNC interrupt_handler

@ --- _interrupt_init --- @

BEGIN_GLOBAL_FUNC TEXT THUMB _interrupt_init
    @ set interrupt vector
    ldr     r0, =0x03007ffc             @ r0 = pointer to master ISR
    ldr     r1, =interrupt_handler
    str     r1, [r0]

    @ IME = 1
    ldr     r0, =0x04000208             @ r0 = pointer to IME
    movs    r1, #1
    str     r1, [r0]

    bx      lr
END_FUNC _interrupt_init

@ --- interrupt_set_isr --- @

@ input:
@   r0 = id  : i32
@   r1 = isr : function pointer
BEGIN_GLOBAL_FUNC TEXT ARM interrupt_set_isr
    @ check if the interrupt ID is valid
    cmp     r0, #INTERRUPT_COUNT

    @ isr_table[id] = id
    ldrlo   r2, =isr_table
    strlo   r1, [r2, r0, lsl #2]

    bx      lr
END_FUNC interrupt_set_isr

@ --- interrupt_wait --- @

@ input:
@   r0 = id : i32
BEGIN_GLOBAL_FUNC TEXT THUMB interrupt_wait
    @ set arguments of IntrWait
    movs    r1, #1
    lsls    r1, r0                      @ (r1) flags = BIT(id)
    movs    r0, #1                      @ (r0) discard = 1

    @ call IntrWait
    swi     0x04
    bx      lr
END_FUNC interrupt_wait

.end
