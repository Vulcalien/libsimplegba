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

.include "macros.inc"

@ --- memory_copy_32 --- @
.global memory_copy_32
.text

@ This function copies data from one memory area to another, using only
@ 32-bit reads and writes.
@
@ Return value:
@   The 'dest' pointer.
@
@ Constraints:
@ - The two memory areas must not overlap.
@ - The 'dest' and 'src' pointers must be 4-byte aligned.
@
@ Notes:
@ - The lowest 2 bits of 'n' are ignored.
@
@ Implementation:
@   Data is copied left-to-right: at first, in blocks of 4 units at a
@   time for as much as possible; at the end, one unit at a time.
@   By using the 'ldm' and 'stm' instructions, the blocks of 4 units are
@   copied efficiently.

@ input:
@   r0 = dest : void *
@   r1 = src  : const void *
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_FUNC THUMB memory_copy_32
    push    {r0, r4-r7}

    @ calculate number of units and blocks
    lsr     r2, #2                      @ (r2) n /= 4
    lsr     r3, r2, #2                  @ (r3) blocks = n / 4
    beq     .L_exit_block_loop          @ if blocks == 0, skip block loop

.L_block_loop:
    ldmia   r1!, {r4-r7}                @ (r1) src  += 16
    stmia   r0!, {r4-r7}                @ (r0) dest += 16

    sub     r3, #1                      @ (r3) blocks--
    bne     .L_block_loop               @ if blocks != 0, repeat loop
.L_exit_block_loop:

    @ calculate remaining units
    lsl     r2, #30
    lsr     r2, #30                     @ (r2) n %= 4
    beq     .L_exit_single_loop         @ if n == 0, skip single loop

.L_single_loop:
    ldmia   r1!, {r4}                   @ (r1) src  += 4
    stmia   r0!, {r4}                   @ (r0) dest += 4

    sub     r2, #1                      @ (r2) n--
    bne     .L_single_loop              @ if n != 0, repeat loop
.L_exit_single_loop:

    @ return original value of dest
    pop     {r0, r4-r7}
    bx      lr
END_FUNC memory_copy_32

.end
