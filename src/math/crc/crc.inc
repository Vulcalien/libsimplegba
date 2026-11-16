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

.macro GENERATE_CRC_FUNC LENGTH:req LUT:req

@ input:
@   r0 = buffer : const void *
@   r1 = n      : i32
@ output:
@   r0 = result : i32
BEGIN_GLOBAL_FUNC .text.crc THUMB math_crc\LENGTH
    push    {r4}

    ldr     r2, =\LUT                   @ (r2) LUT
    movs    r3, #0                      @ (r3) crc = 0
    @ TODO start with 0xffffffff ?
.L_loop:
    ldrb    r4, [r0]                    @ data = *buffer
    adds    r0, #1                      @ (r0) buffer++
    eors    r4, r3                      @ (r4) index = data ^ crc

    .if \LENGTH <= 8
        lsls    r4, #24
        lsrs    r4, #24
        ldrb    r4, [r2, r4]            @ (r4) entry = LUT[index & 0xff]
    .elseif \LENGTH <= 16
        lsls    r4, #24
        lsrs    r4, #23
        ldrh    r4, [r2, r4]            @ (r4) entry = LUT[index & 0xff]
    .elseif \LENGTH <= 32
        lsls    r4, #24
        lsrs    r4, #22
        ldr     r4, [r2, r4]            @ (r4) entry = LUT[index & 0xff]
    .endif

    @ update CRC value
    lsrs    r3, #8
    eors    r3, r4                      @ (r3) crc = (crc >> 8) ^ entry

    subs    r1, #1                      @ (r1) n--
    bgt     .L_loop                     @ if n > 0, continue
.L_exit_loop:

    @ calculate final result
    mvns    r0, r3                      @ (r0) result = crc ^ 0xffffffff
    .if \LENGTH < 32
        lsls    r0, #(32 - \LENGTH)
        lsrs    r0, #(32 - \LENGTH)     @ (r0) result &= BIT(LENGTH) - 1
    .endif

    pop     {r4}
    bx      lr
END_FUNC math_crc\LENGTH

.endm
