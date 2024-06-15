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

@ TODO: add memcpy8, memcpy16

@ --- memcpy32 --- @
.global memcpy32
.text
ARM_FUNC

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

1: @ multi-copy loop
    @ while n >= 16, copy multiple words
    cmp     r2, #16

    ldmhsia r1!, {r3-r6}                @ src  += 16
    stmhsia r0!, {r3-r6}                @ dest += 16

    subhs   r2, #16                     @ n -= 16
    bhs     1b @ multi-copy loop

2: @ single-copy loop
    @ while n >= 4, copy a single word
    cmp     r2, #4

    ldmhsia r1!, {r3}                   @ src  += 4
    stmhsia r0!, {r3}                   @ dest += 4

    subhs   r2, #4                      @ n -= 4
    bhs     2b @ single-copy loop

    @ return dest
    pop     {r0, r4-r6}
    bx      lr

.size memcpy32, .-memcpy32

.align
.pool

.end
