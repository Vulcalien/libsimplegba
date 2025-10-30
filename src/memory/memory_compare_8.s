@ Copyright 2025 Vulcalien
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

@ --- memory_compare_8 --- @

@ Compare memory areas, using only 8-bit reads.
@
@ Return value:
@ - negative if a < b
@ - zero     if a == b
@ - positive if a > b

@ input:
@   r0 = a : const void *
@   r1 = b : const void *
@   r2 = n : u32
@ output:
@   r0 = diff : i32
BEGIN_GLOBAL_FUNC .text THUMB memory_compare_8
    @ if n == 0, return 0
    cmp     r2, #0
    beq     .L_return_zero

    push    {r4-r5}

    @ move 'a' and 'b' pointers to r4 and r5
    movs    r4, r0                      @ (r4) a
    movs    r5, r1                      @ (r5) b

    @ calculate number of blocks
    lsrs    r3, r2, #2                  @ (r3) blocks = n / 4
    beq     .L_exit_block_loop          @ if blocks == 0, skip block loop

.L_block_loop:
    ldrb    r0, [r4, #0]
    ldrb    r1, [r5, #0]
    subs    r0, r1
    bne     .L_return

    ldrb    r0, [r4, #1]
    ldrb    r1, [r5, #1]
    subs    r0, r1
    bne     .L_return

    ldrb    r0, [r4, #2]
    ldrb    r1, [r5, #2]
    subs    r0, r1
    bne     .L_return

    ldrb    r0, [r4, #3]
    ldrb    r1, [r5, #3]
    subs    r0, r1
    bne     .L_return

    adds    r4, #4                      @ (r4) a += 4
    adds    r5, #4                      @ (r5) b += 4

    subs    r3, #1                      @ (r3) blocks--
    bne     .L_block_loop               @ if blocks != 0, repeat loop
.L_exit_block_loop:

    @ calculate remaining units
    lsls    r2, #30
    lsrs    r2, #30                     @ (r2) n %= 4
    beq     .L_exit_single_loop         @ if n == 0, skip single loop

.L_single_loop:
    ldrb    r0, [r4]
    ldrb    r1, [r5]
    subs    r0, r1
    bne     .L_return

    adds    r4, #1                      @ (r4) a += 1
    adds    r5, #1                      @ (r5) b += 1

    subs    r2, #1                      @ (r2) n--
    bne     .L_single_loop              @ if n != 0, repeat loop
.L_exit_single_loop:

.L_return:
    pop     {r4-r5}
    bx      lr

.L_return_zero:
    movs    r0, #0
    bx      lr
END_FUNC memory_compare_8

.end
