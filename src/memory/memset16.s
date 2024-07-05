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

@ --- memset16 --- @
.global memset16
.text
THUMB_FUNC

@ Fill a memory area with a given byte value: 16-bit version.
@
@ Only the lowest 8 bits of 'byte' will be considered. The highest
@ 24 bits will be ignored.
@
@ 'dest' must be 2-byte aligned.
@ 'n' should be a multiple of 2, but its lowest bit is ignored.

@ input:
@   r0 = dest : pointer
@   r1 = byte : signed 32-bit
@   r2 = n    : unsigned 32-bit
@ output:
@   r0 = dest : pointer
memset16:
    push    {r0}

    @ duplicate 'byte' to fill 16 bits
    lsl     r3, r1, #8                  @ r3 = xx 00
    orr     r1, r3                      @ r1 = xx xx

    @ calculate number of halfwords and 4-halfword blocks
    lsr     r2, #1                      @ (r2) n /= 2
    lsr     r3, r2, #2                  @ (r3) blocks = n / 4

    @ if blocks == 0, skip the multi-store loop
    beq     2f @ exit multi-store loop

1: @ multi-store loop
    strh    r1, [r0, #0]
    strh    r1, [r0, #2]
    strh    r1, [r0, #4]
    strh    r1, [r0, #6]

    add     r0, #8                      @ (r0) dest += 8

    sub     r3, #1                      @ (r3) blocks -= 1
    bne     1b @ multi-store loop       @ if blocks != 0, repeat loop
2: @ exit multi-store loop

    @ calculate n % 4
    lsl     r2, #30
    lsr     r2, #30

    @ if n == 0, skip the single-store loop
    beq     4f @ exit single-store loop

3: @ single-store loop
    strh    r1, [r0]

    add     r0, #2                      @ (r0) dest += 2

    sub     r2, #1                      @ (r2) n -= 1
    bne     3b @ single-store loop      @ if n != 0, repeat loop
4: @ exit single-store loop

    @ return original value of dest
    pop     {r0}
    bx      lr

.size memset16, .-memset16

.end
