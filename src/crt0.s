@ Copyright 2022-2024 Vulcalien
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

@ Based on crt0.S v1.28 by Jeff Frohwein

.include "assembly.inc"

BEGIN_GLOBAL_FUNC .crt0 ARM start_vector
    @ switch to IRQ mode and set SP_irq
    mov     r0, #0x12
    msr     cpsr, r0
    ldr     sp, =__sp_irq

    @ switch to System mode and set SP_usr
    mov     r0, #0x1f
    msr     cpsr, r0
    ldr     sp, =__sp_usr

    @ switch to Thumb
    add     r0, pc, #1
    bx      r0

    .thumb

    @ enable prefetch buffer
    ldr     r0, =0x04000204
    ldr     r1, =0x4000
    strh    r1, [r0]

    @ clear .bss section
    ldr     r0, =__bss_start            @ r0 = dest
    movs    r1, #0                      @ r1 = byte
    ldr     r2, =__bss_size             @ r2 = n
    bl      memory_set_32

    @ clear .sbss section
    ldr     r0, =__sbss_start           @ r0 = dest
    movs    r1, #0                      @ r1 = byte
    ldr     r2, =__sbss_size            @ r2 = n
    bl      memory_set_32

    @ copy .data section
    ldr     r0, =__data_start           @ r0 = dest
    ldr     r1, =__data_lma             @ r1 = src
    ldr     r2, =__data_size            @ r2 = n
    bl      memory_copy_32

    @ copy .iwram section
    ldr     r0, =__iwram_start          @ r0 = dest
    ldr     r1, =__iwram_lma            @ r1 = src
    ldr     r2, =__iwram_size           @ r2 = n
    bl      memory_copy_32

    @ copy .ewram section
    ldr     r0, =__ewram_start          @ r0 = dest
    ldr     r1, =__ewram_lma            @ r1 = src
    ldr     r2, =__ewram_size           @ r2 = n
    bl      memory_copy_32

    @ initialize interrupts
    bl      _interrupt_init

    @ --- call AgbMain --- @

    @ set start_vector as return address
    ldr     r0, =start_vector
    mov     lr, r0

    ldr     r0, =AgbMain
    bx      r0
END_FUNC start_vector

.end
