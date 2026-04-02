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

@ Small wrapper around width-speficic memory_copy_x functions.
@
@ The smallest alignment between 'dest', 'src' and 'n' determines the
@ width-specific function to call. This is not optimal in cases where
@ the values are equally misaligned (e.g. dest=3, src=7), but handling
@ those (rare?) cases would make this function much more complex.

@ input:
@   r0 = dest : void *
@   r1 = src  : const void *
@   r2 = n    : u32
@ output:
@   r0 = dest : void *
BEGIN_GLOBAL_FUNC .text.memcpy THUMB memcpy
    @ tmp = (dest | src | n)
    movs    r3, r0                      @ tmp = dest
    orrs    r3, r1                      @ tmp |= src
    orrs    r3, r2                      @ tmp |= n

    @ if 4-byte aligned, use memory_copy_32
    lsls    r3, #30
    beq     memory_copy_32

    @ if 2-byte aligned, use memory_copy_16
    lsls    r3, #1
    beq     memory_copy_16

    b       memory_copy_8
END_FUNC memcpy

.end
