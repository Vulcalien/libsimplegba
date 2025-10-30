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

@ --- memory_set_8 --- @

@ Fill a memory area with a given byte, using only 8-bit writes.
@
@ Notes:
@ - Only the lowest 8 bits of 'byte' are used.

@ input:
@   r0 = dest : void *
@   r1 = byte : i32
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_GLOBAL_FUNC TEXT THUMB memory_set_8
    push    {r0}

    @ calculate number of blocks
    lsrs    r3, r2, #2                  @ (r3) blocks = n / 4
    beq     .L_exit_block_loop          @ if blocks == 0, skip block loop

.L_block_loop:
    strb    r1, [r0, #0]
    strb    r1, [r0, #1]
    strb    r1, [r0, #2]
    strb    r1, [r0, #3]

    adds    r0, #4                      @ (r0) dest += 4
    subs    r3, #1                      @ (r3) blocks--
    bne     .L_block_loop               @ if blocks != 0, repeat loop
.L_exit_block_loop:

    @ calculate remaining units
    lsls    r2, #30
    lsrs    r2, #30                     @ (r2) n %= 4
    beq     .L_exit_single_loop         @ if n == 0, skip single loop

.L_single_loop:
    strb    r1, [r0]

    adds    r0, #1                      @ (r0) dest += 1
    subs    r2, #1                      @ (r2) n--
    bne     .L_single_loop              @ if n != 0, repeat loop
.L_exit_single_loop:

    @ return original value of dest
    pop     {r0}
    bx      lr
END_FUNC memory_set_8

.end
