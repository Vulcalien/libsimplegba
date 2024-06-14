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

@ --- memset --- @
.global memset
.text
ARM_FUNC

@ This is an implementation of the C standard function 'memset'.
@
@ It can handle any alignment, both at the beginning (with 'dest') and
@ at the end ('dest' + n). These special alignment cases are handled by
@ the 'misaligned_start' and 'misaligned_end' pseudo-functions.
@ The function will write using the largest alignment possible at any
@ time, making it safe to use for VRAM (where 8-bits writes are not
@ supported).
@
@ The highest 24-bits of 'byte' are ignored and overwritten.
@
@ To improve performance, the function writes in blocks of 16 bytes
@ whenever possible.

@ input:
@   r0 = dest : pointer
@   r1 = byte : signed 32-bit
@   r2 = n    : unsigned 32-bit
@ output:
@   r0 = dest : pointer
memset:
    @ copy dest into r3 to preserve the value of r0 (to be returned)
    mov     r3, r0                      @ r3 = dest

    @ duplicate 'byte' to fill 32-bits
    orr     r1, r1, lsl #8              @ r1 = content (16-bit)
    orr     r1, r1, lsl #16             @ r1 = content (32-bit)

    @ if dest is not aligned to 4, handle that special case
    tst     r3, #3
    bne     misaligned_start
misaligned_start_return:

    @ store 4 words per iteration (to reduce loop overhead)
1: @ 4 words loop
    @ if n >= 16, store 4 words
    cmp     r2, #16
    strhs   r1, [r3], #4                @ dest += 4
    strhs   r1, [r3], #4                @ dest += 4
    strhs   r1, [r3], #4                @ dest += 4
    strhs   r1, [r3], #4                @ dest += 4
    subhs   r2, #16                     @ n    -= 16
    bhs     1b @ 4 words loop

    @ store 1 word per iteration
2: @ 1 word loop
    @ if n >= 4, store a word
    cmp     r2, #4
    strhs   r1, [r3], #4                @ dest += 4
    subhs   r2, #4                      @ n    -= 4
    bhs     2b @ 1 word loop

    @ if (dest + n) is not aligned to 4, handle that special case
    tst     r2, #3
    bne     misaligned_end              @ if called, this returns

    bx      lr

@ handle cases where the first bytes are not aligned to 4
misaligned_start:
    @ if n == 0, return
    cmp     r2, #0
    bxeq    lr

    @ if dest is aligned to 1, store a byte
    tst     r3, #1
    strneb  r1, [r3], #1                @ dest += 1
    subne   r2, #1                      @ n    -= 1

    @ if n == 0, return
    cmp     r2, #0
    bxeq    lr

    @ if dest is aligned to 2, store an halfword
    tst     r3, #2
    strneh  r1, [r3], #2                @ dest += 2
    subne   r2, #2                      @ n    -= 2

    b       misaligned_start_return

@ handle cases where the last bytes are not aligned to 4
misaligned_end:
    @ if (n & 2) != 0, store an halfword
    tst     r2, #2
    strneh  r1, [r3], #2                @ dest += 2

    @ (n & 1) != 0, store a byte
    tst     r2, #1
    strneb  r1, [r3], #1                @ dest += 1

    @ return directly
    bx      lr

.size memset, .-memset

.end
