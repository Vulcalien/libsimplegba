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

.equ BACKGROUND_COUNT, 4

@ --- background_config --- @
.global background_config
.text
.thumb
.thumb_func

@ r0 = id
@ r1 = pointer to config (16-bit)
background_config:
    @ check if the given ID is valid
    cmp     r0, #(BACKGROUND_COUNT)
    bhs     255f @ exit

    @ get address of background's control register
    ldr     r2, =0x04000008
    lsl     r0, #1                      @ r0 = id * 2

    @ read and store config value
    ldrh    r1, [r1]                    @ r1 = config
    strh    r1, [r2, r0]                @ BG control = config

255: @ exit
    bx      lr

.align
.pool

@ --- background_set_offset --- @
.global background_set_offset
.text
.thumb
.thumb_func

@ r0 = id
@ r1 = x (16-bit)
@ r2 = y (16-bit)
background_set_offset:
    @ check if the given ID is valid
    cmp     r0, #(BACKGROUND_COUNT)
    bhs     255f @ exit

    @ combine the two 16-bit values into one 32-bit value
    lsl     r2, #16                     @ r2 = y << 16
    orr     r1, r2                      @ r1 = x | y << 16

    @ get address of background's offset register
    ldr     r2, =0x04000010
    lsl     r0, #2                      @ r0 = id * 4

    @ store offset value
    str     r1, [r2, r0]                @ BG offset = x | y << 16

255: @ exit
    bx      lr

.end
