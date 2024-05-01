@ Copyright 2024 Vulcalien
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

.include "macros.inc"

.equ TIMER_COUNT, 4

@ --- timer_config --- @
.global timer_config
.text
THUMB_FUNC

@ r0 = id
@ r1 = pointer to config (16-bit)
timer_config:
    @ check if the given ID is valid
    cmp     r0, #(TIMER_COUNT)
    bhs     255f @ exit

    @ get address of timer's control register
    ldr     r2, =0x04000102
    lsl     r0, #2                      @ r0 = id * 4

    @ read and store config value
    ldrh    r1, [r1]                    @ r1 = config
    strh    r1, [r2, r0]                @ Timer control = config

255: @ exit
    bx      lr

.align
.pool

@ --- timer_stop --- @
.global timer_stop
.text
THUMB_FUNC

@ r0 = id
timer_stop:
    @ check if the given ID is valid
    cmp     r0, #(TIMER_COUNT)
    bhs     255f @ exit

    @ get address of timer's control register
    ldr     r2, =0x04000102
    lsl     r0, #2                      @ r0 = id * 4

    @ disable the timer
    mov     r1, #0
    strh    r1, [r2, r0]                @ Timer control = 0

255: @ exit
    bx      lr

.align
.pool

@ --- timer_get_remaining --- @
.global timer_get_remaining
.text
THUMB_FUNC

@ r0 = id
timer_get_remaining:
    @ check if the given ID is valid
    cmp     r0, #(TIMER_COUNT)
    bhs     255f @ exit

    @ get address of timer's counter/reload register
    ldr     r2, =0x04000100
    lsl     r0, #2                      @ r0 = id * 4

    @ read counter value
    ldrh    r1, [r2, r0]                @ r1 = Timer counter

    @ calculate return value (= (0x10000 - counter) & 0xffff)
    mov     r0, #1
    lsl     r0, #16                     @ r0 = 0x10000
    sub     r3, r0, #1                  @ r3 = 0xffff
    sub     r0, r1                      @ r0 = 0x10000 - counter
    and     r0, r3                  @ r0 = (0x10000 - counter & 0xffff)

255: @ exit
    bx      lr

.align
.pool

@ --- timer_set_remaining --- @
.global timer_set_remaining
.text
THUMB_FUNC

@ r0 = id
@ r1 = remaining (16-bit)
timer_set_remaining:
    @ check if the given ID is valid
    cmp     r0, #(TIMER_COUNT)
    bhs     255f @ exit

    @ get address of timer's counter/reload register
    ldr     r2, =0x04000100
    lsl     r0, #2                      @ r0 = id * 4

    @ calculate reload value (= 0x10000 - remaining)
    mov     r3, #1
    lsl     r3, #16                     @ r3 = 0x10000
    sub     r3, r1                      @ r3 = 0x10000 - remaining

    @ store reload value (as 16-bit)
    strh    r3, [r2, r0]                @ Timer reload = reload

255: @ exit
    bx      lr

.end
