@ Copyright 2024-2025 Vulcalien
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

.macro BEGIN_FUNC isa:req name:req
    .if \isa == ARM
        .arm
        .align 2
    .elseif \isa == THUMB
        .thumb
        .thumb_func
        .align 1
    .else
        .err "selected ISA is neither ARM nor THUMB"
    .endif

    .func \name, \name
    .type \name, %function

    \name\():
.endm

.macro END_FUNC name:req
    .pool
    .size \name, .-\name
    .endfunc
.endm
