@ Copyright 2026 Vulcalien
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

@ determine i-th bit of quotient
.macro STEP i:req
    cmp     r1, r0, lsr #\i             @ if den <= (num >> i):
    subls   r0, r0, r1, lsl #\i         @   (r0) num -= (den << i)
    orrls   r2, r2, #(1 << \i)          @   (r2) quotient |= BIT(i)
.endm

@ input:
@   r0 = num
@   r1 = den
@ output:
@   r0 = quotient
@   r1 = remainder
BEGIN_GLOBAL_FUNC .iwram ARM math_udiv
    mov     r2, #0                      @ (r2) quotient = 0

    .set i, 31
    .rept 32
        STEP(i)
        .set i, i-1
    .endr

    @ return quotient and remainder
    mov     r1, r0                      @ r1 = remainder
    mov     r0, r2                      @ r0 = quotient
    bx      lr
END_FUNC math_udiv

.end
