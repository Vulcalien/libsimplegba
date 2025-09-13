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

@ --- memory_set_16 --- @

@ Fill a memory area with a given byte, using only 16-bit writes.
@
@ Constraints:
@ - The 'dest' pointer must be 2-byte aligned.
@
@ Notes:
@ - Only the lowest 8 bits of 'byte' are used.
@ - The lowest bit of 'n' is ignored.

@ input:
@   r0 = dest : void *
@   r1 = byte : i32
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_GLOBAL_FUNC TEXT THUMB memory_set_16
    push    {r0}

    @ make sure 'byte' is actually 8-bit
    lsl     r1, #24                     @ xx 00 00 00
    lsr     r1, #24                     @ 00 00 00 xx

    @ duplicate 'byte' to fill 16 bits
    lsl     r3, r1, #8                  @ r3 = xx 00
    orr     r1, r3                      @ r1 = xx xx

    @ calculate number of units and blocks
    lsr     r2, #1                      @ (r2) n /= 2
    lsr     r3, r2, #2                  @ (r3) blocks = n / 4
    beq     .L_exit_block_loop          @ if blocks == 0, skip block loop

.L_block_loop:
    strh    r1, [r0, #0]
    strh    r1, [r0, #2]
    strh    r1, [r0, #4]
    strh    r1, [r0, #6]

    add     r0, #8                      @ (r0) dest += 8
    sub     r3, #1                      @ (r3) blocks--
    bne     .L_block_loop               @ if blocks != 0, repeat loop
.L_exit_block_loop:

    @ calculate remaining units
    lsl     r2, #30
    lsr     r2, #30                     @ (r2) n %= 4
    beq     .L_exit_single_loop         @ if n == 0, skip single loop

.L_single_loop:
    strh    r1, [r0]

    add     r0, #2                      @ (r0) dest += 2
    sub     r2, #1                      @ (r2) n--
    bne     .L_single_loop              @ if n != 0, repeat loop
.L_exit_single_loop:

    @ return original value of dest
    pop     {r0}
    bx      lr
END_FUNC memory_set_16

.end
