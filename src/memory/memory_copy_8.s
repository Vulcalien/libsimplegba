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

@ --- memory_copy_8 --- @

@ Copy data from one memory area to another, using only 8-bit reads and
@ writes.
@
@ Constraints:
@ - The two memory areas must not overlap.

@ input:
@   r0 = dest : void *
@   r1 = src  : const void *
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_GLOBAL_FUNC TEXT THUMB memory_copy_8
    push    {r0, r4}

    @ calculate number of blocks
    lsrs    r3, r2, #2                  @ (r3) blocks = n / 4
    beq     .L_exit_block_loop          @ if blocks == 0, skip block loop

.L_block_loop:
    ldrb    r4, [r1, #0]
    strb    r4, [r0, #0]

    ldrb    r4, [r1, #1]
    strb    r4, [r0, #1]

    ldrb    r4, [r1, #2]
    strb    r4, [r0, #2]

    ldrb    r4, [r1, #3]
    strb    r4, [r0, #3]

    adds    r1, #4                      @ (r1) src  += 4
    adds    r0, #4                      @ (r0) dest += 4

    subs    r3, #1                      @ (r3) blocks--
    bne     .L_block_loop               @ if blocks != 0, repeat loop
.L_exit_block_loop:

    @ calculate remaining units
    lsls    r2, #30
    lsrs    r2, #30                     @ (r2) n %= 4
    beq     .L_exit_single_loop         @ if n == 0, skip single loop

.L_single_loop:
    ldrb    r4, [r1]
    strb    r4, [r0]

    adds    r1, #1                      @ (r1) src  += 1
    adds    r0, #1                      @ (r0) dest += 1

    subs    r2, #1                      @ (r2) n--
    bne     .L_single_loop              @ if n != 0, repeat loop
.L_exit_single_loop:

    @ return original value of dest
    pop     {r0, r4}
    bx      lr
END_FUNC memory_copy_8

.end
