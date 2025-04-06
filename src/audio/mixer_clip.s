@ Copyright 2025 Vulcalien
@
@ This program is free software: you can redistribute it and/or modify
@ it under the terms of the GNU General Public License as published by
@ the Free Software Foundation, either version 3 of the License, or
@ (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program.  If not, see <https://www.gnu.org/licenses/>.

.include "macros.inc"

.equ BUFFER_SIZE, 768

@ --- _mixer_clip --- @
.global _mixer_clip
.section .iwram, "ax"

@ register allocation:
@   r0 = buffers
@   r1 = temp_buffers
@   r2 = remaining
@   r3 = samples[0], samples[1]

@ input:
@   r0 = buffers      : i8  [OUTPUT_COUNT][BUFFER_SIZE]
@   r1 = temp_buffers : i16 [BUFFER_SIZE][OUTPUT_COUNT]
BEGIN_FUNC ARM _mixer_clip
    mov     r2, #BUFFER_SIZE            @ (r2) remaining = BUFFER_SIZE
.L_samples_loop:
    @ clip samples[1]
    ldrsh   r3, [r1, #2]                @ (r3) samples[1]
    asr     r3, #5                      @ samples[1] /= VOLUME_LEVELS
    cmp     r3, #-128                   @ ========
    movlt   r3, #-128                   @   clip
    cmp     r3, #127                    @  sample
    movgt   r3, #127                    @ ========
    strb    r3, [r0, #BUFFER_SIZE]      @ *buffers[1] = samples[1]

    @ clip samples[0] and increment pointers
    ldrsh   r3, [r1], #4                @ (r3) samples[0]
    asr     r3, #5                      @ samples[0] /= VOLUME_LEVELS
    cmp     r3, #-128                   @ ========
    movlt   r3, #-128                   @   clip
    cmp     r3, #127                    @  sample
    movgt   r3, #127                    @ ========
    strb    r3, [r0], #1                @ *(buffers[0]++) = samples[0]

    subs    r2, #1                      @ (r2) remaining--
    bgt     .L_samples_loop             @ if remaining > 0, repeat loop
.L_exit_samples_loop:

    bx      lr
END_FUNC _mixer_clip

.end
