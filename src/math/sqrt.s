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

@ --- math_sqrt --- @

@ register allocation:
@   r0 = x, result
@   r1 = low
@   r2 = high
@   r3 = guess
@   r4 = guess_sqr

@ input:
@   r0 = x : u32
@ output:
@   r0 = result : i32
BEGIN_GLOBAL_FUNC .iwram ARM math_sqrt
    push    {r4}

    @ set initial bounds so that every possible result is included
    mov     r1, #0                      @ (r1) low  = 0
    mov     r2, #0x10000                @ (r2) high = U16_MAX + 1

    @ try to shrink bounds using powers of two
    mov     r3, #(1 << 4)               @ (r3) guess = 2^4
    mov     r4, #(1 << 8)               @ (r4) guess_sqr = 2^8
.L_shrink_loop:
    cmp     r4, r0
    movls   r1, r3                      @ if guess_sqr <= x, low  = guess
    movhs   r2, r3                      @ if guess_sqr >= x, high = guess
    bhs     .L_exit_shrink_loop         @ if guess_sqr >= x, break

    @ double the guess, and break the loop if guess_sqr overflows
    lsl     r3, #1                      @ (r3) guess *= 2
    lsls    r4, #2                      @ (r4) guess_sqr *= 4
    bne     .L_shrink_loop              @ if guess_sqr != 0, continue
.L_exit_shrink_loop:

    @ use binary search in range [low, high)
.L_search_loop:
    add     r3, r1, r2
    lsr     r3, #1                      @ (r3) guess = (low + high) / 2
    mul     r4, r3, r3                  @ (r4) guess_sqr = guess * guess

    cmp     r4, r0
    movls   r1, r3                      @ if guess_sqr <= x, low  = guess
    movhs   r2, r3                      @ if guess_sqr >= x, high = guess

    sub     r3, r2, r1                  @ tmp = (high - low)
    cmp     r3, #1
    bhi     .L_search_loop              @ if high - low > 1, continue
.L_exit_search_loop:

    @ return low = floor(sqrt(x))
    mov     r0, r1                      @ (r0) result = low

    pop     {r4}
    bx      lr
END_FUNC math_sqrt

.end
