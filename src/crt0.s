@ Copyright 2022-2023 Vulcalien
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

@ Based on crt0.S v1.28 by Jeff Frohwein

.section .crt0, "ax"

.global start_vector
.arm
start_vector:
        @ Set SP_irq
        mov     r0, #0x12               @ Switch to IRQ Mode
        msr     cpsr, r0
        ldr     sp, =__sp_irq

        @ Set SP_usr
        mov     r0, #0x1f               @ Switch to System Mode
        msr     cpsr, r0
        ldr     sp, =__sp_usr

        @ Switch to Thumb mode
        add     r0, pc, #1
        bx      r0

        .thumb

        @ Clear .sbss section
        ldr     r0, =__sbss_start
        ldr     r1, =__sbss_size
        bl      ClearMem

        @ Clear .bss section
        ldr     r0, =__bss_start
        ldr     r1, =__bss_size
        bl      ClearMem

        @ Copy .data section
        ldr     r0, =__data_start
        ldr     r1, =__data_lma
        ldr     r2, =__data_size
        bl      memcpy32

        @ Copy .ewram section
        ldr     r0, =__ewram_start
        ldr     r1, =__ewram_lma
        ldr     r2, =__ewram_size
        bl      memcpy32

        @ Copy .iwram section
        ldr     r0, =__iwram_start
        ldr     r1, =__iwram_lma
        ldr     r2, =__iwram_size
        bl      memcpy32

        @ Call AgbMain
        mov     r0, #0                  @ int argc
        mov     r1, #0                  @ char *argv[]

        ldr     r2, =start_vector       @ Set start_vector as return address
        mov     lr, r2

        ldr     r2, =AgbMain
        bx      r2

.align
.pool

@ r0 = Start Address
@ r1 = Length
.thumb_func
ClearMem:
        @ Return if Length is 0
        cmp     r1, #0
        beq     2f

        mov     r2, #0
1: @ loop
        stmia   r0!, {r2}
        sub     r1, #4
        bgt     1b                      @ 'bgt' in case Length % 4 != 0

2: @ exit
        bx      lr

.end
