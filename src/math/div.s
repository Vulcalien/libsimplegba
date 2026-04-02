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

@ input:
@   r0 = num : i32
@   r1 = den : i32
@ output:
@   r0 = quotient  : i32
@   r1 = remainder : i32
BEGIN_GLOBAL_FUNC .iwram ARM math_div
    push    {r4, lr}

    @ get abs(num) and abs(den); remember which signs to adjust later
    lsr     r4, r1, #31                 @ (r4) mask[0] = (den < 0)
    cmp     r0, #0                      @ if num < 0:
    neglt   r0, r0                      @   (r0) num = -num
    eorlt   r4, #3                      @   (r4) mask[0..1] ^= 1
    cmp     r1, #0                      @ if den < 0:
    neglt   r1, r1                      @   (r1) den = -den

    @ call math_udiv(abs(num), abs(den))
    bl      math_udiv

    @ adjust signs of quotient and remainder:
    @ - mask[0] = (num * den < 0)
    @ - mask[1] = (num < 0)
    tst     r4, #1                      @ if mask[0]:
    negne   r0, r0                      @   quotient = -quotient
    tst     r4, #2                      @ if mask[1]:
    negne   r1, r1                      @   remainder = -remainder

    pop     {r4, lr}
    bx      lr
END_FUNC math_div

.global __aeabi_idiv
.set __aeabi_idiv, math_div

.global __aeabi_idivmod
.set __aeabi_idivmod, math_div

.end
