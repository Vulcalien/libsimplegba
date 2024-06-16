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

@ --- memcpy32 --- @
.global memcpy32
.text
THUMB_FUNC

@ This function copies data from one memory area to another.
@
@ The two areas must not overlap. All parameters (dest, src, n) must be
@ properly aligned to 4.
@
@ By using the 'ldm' and 'stm' instructions, it is able to copy blocks
@ of 4 words (16 bytes) efficiently.

@ input:
@   r0 = dest : pointer
@   r1 = src  : pointer
@   r2 = n    : unsigned 32-bit
@ output:
@   r0 = dest : pointer
memcpy32:
    push    {r0, r4-r6}

    @ calculate number of 32-bit words
    lsr     r2, #2                      @ n /= 2

    @ if n < 4, skip the multi-copy loop
    cmp     r2, #4
    blo     2f @ exit multi-copy loop

1: @ multi-copy loop
    ldmia   r1!, {r3-r6}                @ src  += 16
    stmia   r0!, {r3-r6}                @ dest += 16

    sub     r2, #4                      @ n -= 4

    @ if n >= 4, repeat loop
    cmp     r2, #4
    bhs     1b @ multi-copy loop
2: @ exit multi-copy loop

    @ if n == 0, skip the single-copy loop
    cmp     r2, #0
    beq     4f @ exit single-copy loop

3: @ single-copy loop
    ldmia   r1!, {r3}                   @ src  += 4
    stmia   r0!, {r3}                   @ dest += 4

    sub     r2, #1                      @ n -= 1
    bne     3b @ sigle-copy loop        @ if n != 0, repeat loop
4: @ exit single-copy loop

    @ return dest
    pop     {r0, r4-r6}
    bx      lr

.size memcpy32, .-memcpy32

.align
.pool

.end
