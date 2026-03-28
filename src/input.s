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

.equ KEY_INPUT, 0x04000130

.equ COUNT, 10

@ --- support variables --- @
.bss
.align 2

.global _input_is_down
_input_is_down:
    .space 2

.global _input_was_down
_input_was_down:
    .space 2

.global _input_repeated
_input_repeated:
    .space 2

.global _input_repeat_delay
_input_repeat_delay:
    .space 1

.global _input_repeat_interval
_input_repeat_interval:
    .space 1

time_down:
    .space COUNT

@ --- input_update --- @

@ register allocation:
@   r0 = base of variables
@   r1 = is_down
@   r2 = repeat_delay
@   r3 = repeat_interval
@   r4 = repeated
@   r5 = key_bit
@   r6 = time

BEGIN_GLOBAL_FUNC .text THUMB input_update
    push    {r4-r6}
    ldr     r0, =_input_is_down         @ (r0) base of variables

    @ update _input_was_down
    ldrh    r2, [r0, #0]                @ tmp = _input_is_down
    strh    r2, [r0, #2]                @ _input_was_down = tmp

    @ update _input_is_down
    ldr     r1, =KEY_INPUT
    ldrh    r1, [r1]                    @ (r1) is_down = KEY_INPUT
    mvns    r1, r1                      @ (r1) is_down ^= 0xffff
    strh    r1, [r0, #0]                @ _input_is_down = is_down

@ ==================================================================== @
@                        update _input_repeated                        @
@ ==================================================================== @

    @ load repeat delay, repeat interval and repeated
    ldrb    r2, [r0, #6]                @ (r2) repeat_delay = _input_repeat_delay
    ldrb    r3, [r0, #7]                @ (r3) repeat_interval = _input_repeat_interval
    ldrb    r4, [r0, #4]                @ (r4) repeated = _input_repeated

    @ iterate from (COUNT - 1) down to 0
    movs    r5, #1
    lsls    r5, #(COUNT - 1)            @ (r5) key_bit = BIT(COUNT - 1)
.L_repeated_loop:
    @ if key is not down, reset time and repeated bit
    tst     r1, r5                      @ if (is_down & key_bit) != 0:
    bne     1f                          @   skip early continue
    bics    r4, r5                      @ (r4) repeated &= ~key_bit
    movs    r6, #0                      @ (r6) time = 0
    b       .L_continue_repeated_loop   @ continue
1: @ skip early continue

    @ load and increment time down
    ldrb    r6, [r0, #8 + COUNT - 1]    @ (r6) time = time_down[key]
    adds    r6, #1                      @ (r6) time++

    @ based on time down, check if key is repeated
    cmp     r6, r2                      @ if time > repeat_delay:
    bhi     .L_time_hi_delay            @   jump to time_hi_delay
    cmp     r6, #1                      @ else if time == 1:
    beq     .L_time_eq_1                @   jump to time_eq_1
                                        @ else:
    bics    r4, r5                      @   (r4) repeated &= ~key_bit

.L_continue_repeated_loop:
    strb    r6, [r0, #8 + COUNT - 1]    @ time_down[key] = time
    subs    r0, #1                      @ move base of variables left
    lsrs    r5, #1                      @ (r5) key_bit >>= 1
    bne     .L_repeated_loop            @ if key_bit != 0, repeat loop
.L_exit_repeated_loop:

    @ update _input_repeated
    adds    r0, #COUNT                  @ restore base of variables
    strh    r4, [r0, #4]                @ _input_repeated = repeated

    pop     {r4-r6}
    bx      lr

.L_time_hi_delay:
    subs    r6, r3                      @ (r6) time -= repeat_interval
.L_time_eq_1:
    orrs    r4, r5                      @ (r4) repeated |= key_bit
    b       .L_continue_repeated_loop
END_FUNC input_update

.end
