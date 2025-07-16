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
.equ OUTPUT_COUNT,  2
.equ BUFFER_SIZE,   768

.equ STRUCT_CHANNEL_SIZE, 24

@ --- temp_buffers --- @
.section .sbss, "aw", %nobits
.align 2

@ i16 temp_buffers[sample][output]
temp_buffers:
    .space (BUFFER_SIZE * OUTPUT_COUNT * 2)
.size temp_buffers, .-temp_buffers

@ --- _mixer_update --- @
.global _mixer_update
.section .iwram, "ax", %progbits

@ register allocation:
@   r0  = channels, channel (struct pointer)
@   r1  = buffers
@   r2  = length
@   r3  = c (channel index)
@   r4  = data
@   r5  = end
@   r6  = position
@   r7  = increment
@   r8  = volume
@   r9  = temp_buffers
@   r10 = remaining
@   r11 = sample
@   r12 = loop_length

@ input:
@   r0 = channels : struct Channel [CHANNEL_COUNT]
@   r1 = buffers  : i8 [OUTPUT_COUNT][BUFFER_SIZE]
@   r2 = length   : u32 in range [4, BUFFER_SIZE], multiple of 4
BEGIN_FUNC ARM _mixer_update
    push    {r4-r12}

@ ==================================================================== @
@                          clear temp_buffers                          @
@ ==================================================================== @

    mov     r3, #0
    mov     r4, #0
    mov     r5, #0
    mov     r6, #0

    ldr     r9, =temp_buffers           @ (r9) temp_buffers
    mov     r10, r2                     @ (r10) remaining = length
.L_clear_loop:
    stm     r9!, {r3-r6}

    subs    r10, #4                     @ (r10) remaining -= 4
    bgt     .L_clear_loop               @ if remaining > 0, repeat loop
.L_exit_clear_loop:

@ ==================================================================== @
@                          fill temp_buffers                           @
@ ==================================================================== @

    mov     r3, #(CHANNEL_COUNT - 1)    @ (r3) c = CHANNEL_COUNT - 1
    mov     r4, #STRUCT_CHANNEL_SIZE
    mla     r0, r3, r4, r0              @ (r0) channel = &channels[c]
.L_channels_loop:
    @ retrieve data pointer; if NULL, continue
    ldr     r4, [r0, #0]                @ (r4) data
    cmp     r4, #0
    beq     .L_continue_channels_loop

    @ if channel is paused, continue
    ldrb    r5, [r0, #18]               @ (r5) paused
    cmp     r5, #0
    bne     .L_continue_channels_loop

    @ retrieve end pointer, position and increment
    ldr     r5, [r0, #4]                @ (r5) end
    ldrh    r6, [r0, #8]                @ (r6) position
    ldrh    r7, [r0, #10]               @ (r7) increment

    @ retrieve volume vector
    ldrh    r8, [r0, #20]               @ 00 00 LL RR
    orr     r8, r8, lsl #8              @ 00 LL xx RR
    bic     r8, #0x0000ff00             @ (r8) volume (00 LL 00 RR)

    @ reset temp_buffers and remaining
    ldr     r9, =temp_buffers           @ (r9) temp_buffers
    mov     r10, r2                     @ (r10) remaining = length

    @ calculate data pointer after processing
    mla     r11, r7, r2, r6             @ tmp = pos + inc * len
    add     r11, r4, r11, lsr #12       @ tmp = data + tmp / 0x1000

    @ call most efficient mix_channel pseudo-function given conditions
    cmp     r11, r5                     @ check if tmp < end
    blo     1f                          @ if tmp < end, not near end
    cmp     r7, #0x1000                 @ check if increment == 0x1000
    beq     mix_channel_near_end        @ no pitch + near end
    b       mix_channel_pitch_near_end  @ pitch + near end
1: @ not near end
    cmp     r7, #0x1000                 @ check if increment == 0x1000
    beq     mix_channel                 @ no pitch + not near end
    b       mix_channel_pitch           @ pitch + not near end
.L_mix_channel_return:

    @ write data pointer and position
    str     r4, [r0, #0]                @ channel->data = data
    strh    r6, [r0, #8]                @ channel->position = position

.L_continue_channels_loop:
    sub     r0, #STRUCT_CHANNEL_SIZE    @ (r0) channel--
    subs    r3, #1                      @ (r3) c--
    bge     .L_channels_loop            @ if c >= 0, repeat loop
.L_exit_channels_loop:

@ ==================================================================== @
@                  copy clipped samples into buffers                   @
@ ==================================================================== @

    ldr     r9, =temp_buffers           @ (r9) temp_buffers
.L_clip_loop:
    @ clip sample 1
    ldrsh   r11, [r9, #2]               @ (r11) sample
    asr     r11, #5                     @ sample /= VOLUME_LEVELS
    cmp     r11, #-128                  @ ========
    movlt   r11, #-128                  @   clip
    cmp     r11, #127                   @  sample
    movgt   r11, #127                   @ ========
    strb    r11, [r1, #BUFFER_SIZE]     @ *buffers[1] = sample

    @ clip sample 0
    ldrsh   r11, [r9], #4               @ (r11) sample
    asr     r11, #5                     @ sample /= VOLUME_LEVELS
    cmp     r11, #-128                  @ ========
    movlt   r11, #-128                  @   clip
    cmp     r11, #127                   @  sample
    movgt   r11, #127                   @ ========
    strb    r11, [r1], #1               @ *(buffers[0]++) = sample

    subs    r2, #1                      @ (r2) length--
    bgt     .L_clip_loop                @ if length > 0, repeat loop
.L_exit_clip_loop:

    pop     {r4-r12}
    bx      lr

@ ==================================================================== @
@                     mix channel pseudo-functions                     @
@ ==================================================================== @

.macro MIX_CHANNEL pitch:req near_end:req
1: @ samples loop
    @ get sample and advance position
    .if \pitch == 1
        @ retrieve samples
        ldrsb   r11, [r4]               @ left  = *(data)
        ldrsb   r12, [r4, #1]           @ right = *(data + 1)

        @ interpolate samples
        sub     r12, r11                @ delta = right - left
        mul     r12, r6                 @ delta * position
        add     r11, r12, asr #12       @ (r11) sample = left + delta * position / 0x1000

        @ advance position
        add     r6, r7                  @ (r6) position += increment
        add     r4, r6, lsr #12         @ (r4) data += position / 0x1000
        lsl     r6, #20
        lsr     r6, #20                 @ (r6) position %= 0x1000
    .else
        ldrsb   r11, [r4], #1           @ (r11) sample = *(data++)
    .endif

    @ Add sample to temp_buffers: instead of working on two 16-bit
    @ values separately, work on them combined into a 32-bit value, in a
    @ way similar to vector arithmetic. The driver must be setup so that
    @ 16-bit overflow does not happen, or else artifacts would appear.
    @
    @ Note that treating 32-bit integers as 16-bit vectors sometimes
    @ introduces calculation errors in the top value.
    @
    @ If sample is negative and the right volume is not zero, the high
    @ 16-bit value of (sample * volume) will be off by one. This noise
    @ is particularly noticeable when sound is only playing on the right
    @ output. To compensate for this, one is added to the high 16-bit
    @ value if (sample * volume) is negative, ignoring the case in which
    @ right_volume == 0 but left_volume != 0 for performance reasons.
    @
    @ When adding (sample * volume) to the temporary buffer's value, if
    @ the resulting low 16-bit value overflows, the high 16-bit value
    @ will be off by one. This noise should be hardly noticeable, so no
    @ attempt is made to compensate for it.
    ldr     r12, [r9]                   @ tmp = *temp_buffers
    muls    r11, r8                     @ sample * volume
    addlt   r11, #0x00010000            @ if (sample * volume) < 0, fix top value
    add     r12, r11                    @ tmp += sample * volume
    str     r12, [r9], #4               @ *(temp_buffers++) = tmp

    .if \near_end == 1
        @ if data >= end, loop or stop the channel
        cmp     r4, r5
        bge     3f @ loop or stop
    .endif

2: @ continue samples loop
    subs    r10, #1                     @ (r10) remaining--
    bgt     1b @ samples loop           @ if remaining > 0, repeat loop
    b       .L_mix_channel_return       @ otherwise, return

3: @ loop or stop
    .if \near_end == 1
        @ reset position
        mov     r6, #0                  @ (r6) position = 0

        @ retrieve loop_length
        ldr     r12, [r0, #12]          @ (r12) loop_length
        cmp     r12, #0

        @ if loop_length != 0, loop channel and continue samples loop
        subne   r4, r5, r12             @ (r4) data = end - loop_length
        bne     2b @ continue samples loop

        @ otherwise, stop channel and break samples loop
        mov     r4, #0                  @ (r4) data = NULL
        b       .L_mix_channel_return
    .endif
.endm

mix_channel:
    MIX_CHANNEL 0 0

mix_channel_pitch:
    MIX_CHANNEL 1 0

mix_channel_near_end:
    MIX_CHANNEL 0 1

mix_channel_pitch_near_end:
    MIX_CHANNEL 1 1

END_FUNC _mixer_update

.end
