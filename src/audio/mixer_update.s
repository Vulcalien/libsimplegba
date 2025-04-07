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
@   r2  = c (channel index)
@   r3  = channel (struct pointer)
@   r4  = data
@   r5  = end
@   r6  = position
@   r7  = increment
@   r8  = volume
@   r9  = remaining
@   r10 = sample
@   r11 = loop_length

@ input:
@   r0 = channels     : struct Channel *
@   r1 = temp_buffers : i16 [BUFFER_SIZE][OUTPUT_COUNT]
BEGIN_FUNC ARM _mixer_update
    push    {r4-r11}

@ ==================================================================== @
@                             clear buffer                             @
@ ==================================================================== @

    mov     r2, #0
    mov     r3, #0
    mov     r4, #0
    mov     r5, #0

    mov     r6, r1                      @ (r6) to_clear = temp_buffers
    mov     r9, #BUFFER_SIZE            @ (r9) remaining = BUFFER_SIZE
.L_clear_loop:
    stm     r6!, {r2-r5}

    subs    r9, #4                      @ (r9) remaining -= 4
    bgt     .L_clear_loop               @ if remaining > 0, repeat loop
.L_exit_clear_loop:

@ ==================================================================== @
@                             fill buffer                              @
@ ==================================================================== @

    mov     r2, #(CHANNEL_COUNT - 1)    @ (r2) c = CHANNEL_COUNT - 1
.L_channels_loop:
    @ get pointer to channel's struct
    mov     r3, #24
    mla     r3, r2, r3, r0              @ (r3) channel = &channels[c]

    @ retrieve data pointer; if NULL, continue
    ldr     r4, [r3, #0]                @ (r4) data
    cmp     r4, #0
    beq     .L_continue_channels_loop

    @ if channel is paused, continue
    ldrb    r5, [r3, #18]               @ (r5) paused
    cmp     r5, #0
    bne     .L_continue_channels_loop

    @ retrieve end pointer, position and increment
    ldr     r5, [r3, #4]                @ (r5) end
    ldrh    r6, [r3, #8]                @ (r6) position
    ldrh    r7, [r3, #10]               @ (r7) increment

    @ retrieve volume vector
    ldrh    r8, [r3, #20]               @ 00 00 RR LL
    orr     r8, r8, lsl #8              @ 00 RR xx LL
    bic     r8, #0x0000ff00             @ (r8) volume (00 RR 00 LL)

    @ save value of temp_buffers
    push    {r1}

    mov     r9, #BUFFER_SIZE            @ (r9) remaining = BUFFER_SIZE
.L_samples_loop:
    @ retrieve sample and update position
    ldrsb   r10, [r4]                   @ (r10) sample = *data
    add     r6, r7                      @ (r6) position += increment
    add     r4, r6, lsr #12             @ (r4) data += position / 0x1000
    bic     r6, #0x000ff000             @ (r6) position &= 0xfff

    @ Add sample to temp_buffers: instead of working on two 16-bit
    @ values separately, work on them combined into a 32-bit value, in a
    @ way similar to vector arithmetic. The driver must be setup so that
    @ 16-bit overflow does not happen, or else artifacts would appear.
    @
    @ Note that, if sample is negative, the top 16-bit value is off by
    @ one, so it should be adjusted to prevent noise.
    ldr     r11, [r1]
    muls    r10, r8                     @ sample * volume
    add     r11, r10                    @ tmp += sample * volume
    addlt   r11, #0x00010000            @ if sample < 0, fix top value
    str     r11, [r1], #4

    @ if data >= end, loop or stop the channel
    cmp     r4, r5
    bge     .L_loop_or_stop

.L_continue_samples_loop:
    subs    r9, #1                      @ (r9) remaining--
    bgt     .L_samples_loop             @ if remaining > 0, repeat loop
.L_exit_samples_loop:

    @ restore temp_buffers, and write back data pointer and position
    pop     {r1}                        @ restore temp_buffers
    str     r4, [r3, #0]                @ channel->data = data
    strh    r6, [r3, #8]                @ channel->position = position

.L_continue_channels_loop:
    subs    r2, #1                      @ (r2) c--
    bge     .L_channels_loop            @ if c >= 0, repeat loop
.L_exit_channels_loop:

    pop     {r4-r11}
    bx      lr

@ ==================================================================== @
@                             loop or stop                             @
@ ==================================================================== @

.L_loop_or_stop:
    @ reset position
    mov     r6, #0                      @ (r6) position = 0

    @ retrieve loop_length
    ldr     r11, [r3, #12]              @ (r11) loop_length
    cmp     r11, #0

    @ if loop_length != 0, loop channel and continue sample loop
    subne   r4, r5, r11                 @ (r4) data = end - loop_length
    bne     .L_continue_samples_loop

    @ if loop_length == 0, stop channel and break sample loop
    moveq   r4, #0                      @ (r4) data = NULL
    beq     .L_exit_samples_loop
END_FUNC _mixer_update

.end
