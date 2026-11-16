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

.macro GENERATE_CRC_FUNC WIDTH:req LUT:req

.equ INITIAL_VALUE, ((1 << \WIDTH) - 1)

@ input:
@   r0 = buffer : const void *
@   r1 = n      : i32
@ output:
@   r0 = result : i32
BEGIN_GLOBAL_FUNC .text.crc THUMB math_crc\WIDTH
    push    {r4}

    @ set initial value
    .if INITIAL_VALUE <= 0xff
        movs    r2, #INITIAL_VALUE      @ (r2) crc = INITIAL_VALUE
    .else
        ldr     r2, =INITIAL_VALUE      @ (r2) crc = INITIAL_VALUE
    .endif

    ldr     r3, =\LUT                   @ (r3) LUT
.L_loop:
    ldrb    r4, [r0]                    @ data = *buffer
    adds    r0, #1                      @ (r0) buffer++
    eors    r4, r2                      @ (r4) index = data ^ crc

    .if \WIDTH <= 8
        ldrb    r2, [r3, r4]            @ (r2) crc = LUT[index]
    .elseif \WIDTH <= 16
        lsls    r4, #24
        lsrs    r4, #23
        ldrh    r4, [r3, r4]            @ (r4) entry = LUT[index & 0xff]
        lsrs    r2, #8
        eors    r2, r4                  @ (r2) crc = (crc >> 8) ^ entry
    .elseif \WIDTH <= 32
        lsls    r4, #24
        lsrs    r4, #22
        ldr     r4, [r3, r4]            @ (r4) entry = LUT[index & 0xff]
        lsrs    r2, #8
        eors    r2, r4                  @ (r2) crc = (crc >> 8) ^ entry
    .endif

    subs    r1, #1                      @ (r1) n--
    bgt     .L_loop                     @ if n > 0, continue
.L_exit_loop:

    @ calculate final result
    mvns    r0, r2                      @ (r0) result = crc ^ 0xffffffff
    .if \WIDTH < 32
        lsls    r0, #(32 - \WIDTH)
        lsrs    r0, #(32 - \WIDTH)      @ (r0) result &= BIT(WIDTH) - 1
    .endif

    pop     {r4}
    bx      lr
END_FUNC math_crc\WIDTH

.endm
