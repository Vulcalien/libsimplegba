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

@ Fill a memory area with a given byte value. This is an implementation
@ of the C standard function 'memset'.
@
@ Only the lowest 8 bits of 'byte' will be considered. The highest
@ 24 bits will be ignored.
@
@ Any alignment for 'dest' and 'n' is allowed. The function relies on
@ 'memory_set_32' for writing to memory 4-byte aligned. The rest of the
@ code handles unaligned writes at the start or end.
@
@ Writes will be always of the highest size allowed by the alignment of
@ 'dest' and remaining 'n' bytes.

@ input:
@   r0 = dest : void *
@   r1 = byte : i32
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_FUNC THUMB memset
    mov     r3, lr                      @ r3 = lr
    push    {r0, r3}

    @ make sure 'byte' is actually 8-bit
    lsl     r1, #24                     @ xx 00 00 00
    lsr     r1, #24                     @ 00 00 00 xx

    @ if dest is unaligned, handle that case
    mov     r3, #3                      @ (r3) mask = 3
    tst     r0, r3
    bne     16f @ unaligned start
1: @ unaligned start return

    @ if n % 4 != 0, handle that case
    mov     r3, #3
    tst     r2, r3
    bne     32f @ unaligned end
2: @ unaligned end return

    bl      memory_set_32

    @ return original value of dest
    pop     {r0, r3}
    bx      r3                          @ r3 = original value of lr

@ - unaligned start - @
@ handle cases where the first bytes are unaligned
16: @ unaligned start
    @ if n < 1, return
    cmp     r2, #1
    blo     1b @ unaligned start return

    @ check if (dest & 1) != 0
    mov     r3, #1                      @ (r3) mask = 1
    tst     r0, r3
    beq     17f @ after byte-store

    @ store a byte
    strb    r1, [r0]                    @ *dest = 8-bit content
    add     r0, #1                      @ (r0) dest += 1
    sub     r2, #1                      @ (r2) n -= 1
17: @ after byte-store

    @ if n < 2, return
    cmp     r2, #2
    blo     1b @ unaligned start return

    @ check if (dest & 2) != 0
    mov     r3, #2                      @ (r3) mask = 2
    tst     r0, r3
    beq     1b @ unaligned start return

    @ duplicate 'byte' to fill 16 bits
    lsl     r3, r1, #8                  @ r3 = xx 00
    orr     r1, r3                      @ r1 = xx xx

    @ store an halfword
    strh    r1, [r0]                    @ *dest = 16-bit content
    add     r0, #2                      @ (r0) dest += 2
    sub     r2, #2                      @ (r2) n -= 2

    @ return
    b       1b @ unaligned start return

@ - unaligned end - @
@ handle cases where the last bytes are unaligned
32: @ unaligned end
    @ check if (n & 1) != 0
    mov     r3, #1
    tst     r2, r3
    beq     33f @ after byte-store

    @ store a byte
    sub     r2, #1                      @ (r2) n -= 1
    strb    r1, [r0, r2]                @ dest[n] = 8-bit content
33: @ after byte-store

    @ check if (n & 2) != 0
    mov     r3, #2
    tst     r2, r3
    beq     2b @ unaligned end return

    @ duplicate 'byte' to fill 16 bits
    lsl     r3, r1, #8                  @ r3 = xx 00
    orr     r1, r3                      @ r1 = xx xx

    @ store an halfword
    sub     r2, #2                      @ (r2) n -= 2
    strh    r1, [r0, r2]                @ dest[n] = 16-bit content

    @ return
    b       2b @ unaligned end return
END_FUNC memset

.end
