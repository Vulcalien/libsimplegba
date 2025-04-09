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

.equ CHANNEL_COUNT, 8
.equ BUFFER_SIZE, 768

@ --- _mixer_update --- @
.global _mixer_update
.section .iwram, "ax"

@ register allocation:
@   r0  = channels
@   r1  = temp_buffers
@   r3  = c (channel index)
@   r4  = channel (struct pointer)
@   r5  = data
@   r6  = end
@   r7  = position
@   r8  = increment
@   r9  = volume
@   r10 = remaining
@   r11 = sample
@   r12 = loop_length

@ input:
@   r0 = channels     : struct Channel *
@   r1 = temp_buffers : i16 [BUFFER_SIZE][OUTPUT_COUNT]
BEGIN_FUNC ARM _mixer_update
    push    {r4-r12}

@ ==================================================================== @
@                             clear buffer                             @
@ ==================================================================== @

    mov     r3, #0
    mov     r4, #0
    mov     r5, #0
    mov     r6, #0

    mov     r7, r1                      @ (r7) to_clear = temp_buffers
    mov     r10, #BUFFER_SIZE           @ (r10) remaining = BUFFER_SIZE
.L_clear_loop:
    stm     r7!, {r3-r6}

    subs    r10, #4                     @ (r10) remaining -= 4
    bgt     .L_clear_loop               @ if remaining > 0, repeat loop
.L_exit_clear_loop:

@ ==================================================================== @
@                             fill buffer                              @
@ ==================================================================== @

    mov     r3, #(CHANNEL_COUNT - 1)    @ (r3) c = CHANNEL_COUNT - 1
.L_channels_loop:
    @ get pointer to channel's struct
    mov     r4, #24
    mla     r4, r3, r4, r0              @ (r4) channel = &channels[c]

    @ retrieve data pointer; if NULL, continue
    ldr     r5, [r4, #0]                @ (r5) data
    cmp     r5, #0
    beq     .L_continue_channels_loop

    @ if channel is paused, continue
    ldrb    r6, [r4, #18]               @ (r6) paused
    cmp     r6, #0
    bne     .L_continue_channels_loop

    @ retrieve end pointer, position and increment
    ldr     r6, [r4, #4]                @ (r6) end
    ldrh    r7, [r4, #8]                @ (r7) position
    ldrh    r8, [r4, #10]               @ (r8) increment

    @ retrieve volume vector
    ldrh    r9, [r4, #20]               @ 00 00 RR LL
    orr     r9, r9, lsl #8              @ 00 RR xx LL
    bic     r9, #0x0000ff00             @ (r9) volume (00 RR 00 LL)

    @ save value of temp_buffers
    push    {r1}

    mov     r10, #BUFFER_SIZE           @ (r10) remaining = BUFFER_SIZE
.L_samples_loop:
    @ retrieve sample and update position
    ldrsb   r11, [r5]                   @ (r11) sample = *data
    add     r7, r8                      @ (r7) position += increment
    add     r5, r7, lsr #12             @ (r5) data += position / 0x1000
    bic     r7, #0x000ff000             @ (r7) position &= 0xfff

    @ Add sample to temp_buffers: instead of working on two 16-bit
    @ values separately, work on them combined into a 32-bit value, in a
    @ way similar to vector arithmetic. The driver must be setup so that
    @ 16-bit overflow does not happen, or else artifacts would appear.
    @
    @ Note that, if sample is negative, the top 16-bit value is off by
    @ one, so it should be adjusted to prevent noise.
    ldr     r12, [r1]
    muls    r11, r9                     @ sample * volume
    add     r12, r11                    @ tmp += sample * volume
    addlt   r12, #0x00010000            @ if sample < 0, fix top value
    str     r12, [r1], #4

    @ if data >= end, loop or stop the channel
    cmp     r5, r6
    bge     .L_loop_or_stop

.L_continue_samples_loop:
    subs    r10, #1                     @ (r10) remaining--
    bgt     .L_samples_loop             @ if remaining > 0, repeat loop
.L_exit_samples_loop:

    @ restore temp_buffers, and write back data pointer and position
    pop     {r1}                        @ restore temp_buffers
    str     r5, [r4, #0]                @ channel->data = data
    strh    r7, [r4, #8]                @ channel->position = position

.L_continue_channels_loop:
    subs    r3, #1                      @ (r3) c--
    bge     .L_channels_loop            @ if c >= 0, repeat loop
.L_exit_channels_loop:

    pop     {r4-r12}
    bx      lr

@ ==================================================================== @
@                             loop or stop                             @
@ ==================================================================== @

.L_loop_or_stop:
    @ reset position
    mov     r7, #0                      @ (r7) position = 0

    @ retrieve loop_length
    ldr     r12, [r4, #12]              @ (r12) loop_length
    cmp     r12, #0

    @ if loop_length != 0, loop channel and continue sample loop
    subne   r5, r6, r12                 @ (r5) data = end - loop_length
    bne     .L_continue_samples_loop

    @ if loop_length == 0, stop channel and break sample loop
    moveq   r5, #0                      @ (r5) data = NULL
    beq     .L_exit_samples_loop
END_FUNC _mixer_update

.end
