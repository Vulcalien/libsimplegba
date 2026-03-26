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
@   r0 = num : u32
@   r1 = den : u32
@ output:
@   r0 = quotient  : u32
@   r1 = remainder : u32
BEGIN_GLOBAL_FUNC .iwram ARM math_udiv
    mov     r2, #0                      @ (r2) quotient = 0

    @ based on size of num, skip unnecessary steps
    cmp     r0, #(1 << 8)
    blo     .L_8bit
    cmp     r0, #(1 << 16)
    blo     .L_16bit
    cmp     r0, #(1 << 24)
    blo     .L_24bit

.L_32bit:
    STEP(31)
    STEP(30)
    STEP(29)
    STEP(28)
    STEP(27)
    STEP(26)
    STEP(25)
    STEP(24)
.L_24bit:
    STEP(23)
    STEP(22)
    STEP(21)
    STEP(20)
    STEP(19)
    STEP(18)
    STEP(17)
    STEP(16)
.L_16bit:
    STEP(15)
    STEP(14)
    STEP(13)
    STEP(12)
    STEP(11)
    STEP(10)
    STEP(9)
    STEP(8)
.L_8bit:
    STEP(7)
    STEP(6)
    STEP(5)
    STEP(4)
    STEP(3)
    STEP(2)
    STEP(1)
    STEP(0)

    @ return quotient and remainder
    mov     r1, r0                      @ r1 = remainder
    mov     r0, r2                      @ r0 = quotient
    bx      lr
END_FUNC math_udiv

.end
