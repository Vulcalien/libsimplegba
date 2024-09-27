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

@ --- memcpy16 --- @
.global memcpy16
.text
THUMB_FUNC

@ This function copies data from one memory area to another, using only
@ 16-bit reads and writes.
@
@ Return value:
@   The 'dest' pointer.
@
@ Constraints:
@ - The two memory areas must not overlap.
@ - The 'dest' and 'src' pointers must be 2-byte aligned.
@
@ Notes:
@ - The lowest bit of 'n' is ignored.
@
@ Implementation:
@   Data is copied left-to-right: at first, in blocks of 4 units at a
@   time for as much as possible; at the end, one unit at a time.

@ input:
@   r0 = dest : pointer
@   r1 = src  : pointer
@   r2 = n    : unsigned 32-bit
@ output:
@   r0 = dest : pointer
memcpy16:
    push    {r0, r4}

    @ calculate number of halfwords and 4-halfword blocks
    lsr     r2, #1                      @ (r2) n /= 2
    lsr     r3, r2, #2                  @ (r3) blocks = n / 4

    @ if blocks == 0, skip the multi-copy loop
    beq     2f @ exit multi-copy loop

1: @ multi-copy loop
    ldrh    r4, [r1, #0]
    strh    r4, [r0, #0]

    ldrh    r4, [r1, #2]
    strh    r4, [r0, #2]

    ldrh    r4, [r1, #4]
    strh    r4, [r0, #4]

    ldrh    r4, [r1, #6]
    strh    r4, [r0, #6]

    add     r1, #8                      @ (r1) src  += 8
    add     r0, #8                      @ (r0) dest += 8

    sub     r3, #1                      @ (r3) blocks -= 1
    bne     1b @ multi-copy loop        @ if blocks != 0, repeat loop
2: @ exit multi-copy loop

    @ calculate n % 4
    lsl     r2, #30
    lsr     r2, #30

    @ if n == 0, skip the single-copy loop
    beq     4f @ exit single-copy loop

3: @ single-copy loop
    ldrh    r4, [r1]
    strh    r4, [r0]

    add     r1, #2                      @ (r1) src  += 2
    add     r0, #2                      @ (r0) dest += 2

    sub     r2, #1                      @ (r2) n -= 1
    bne     3b @ sigle-copy loop        @ if n != 0, repeat loop
4: @ exit single-copy loop

    @ return original value of dest
    pop     {r0, r4}
    bx      lr

.size memcpy16, .-memcpy16

.end
