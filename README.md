# libsimplegba
A library containing utilities for the Game Boy Advance.

## Purpose
*libsimplegba* is a collection of functions and constants that can be
used to interface with the GBA hardware hiding some of the complexity
and low-level details.

The library focuses on ease of use without sacrificing efficiency: most
of the functions are implemented as *inline functions*, meaning that
under most circumstances they are just as efficient as writing to the
hardware registers directly.

## Modules
The following modules are available:
| Category | Module     | Description                                  |
| :------: | ---------- | -------------------------------------------- |
| Misc     | Base       | basic definitions (types, macros...)         |
| -        | Math       | various mathematical functions               |
| -        | Random     | pseudo-random number generator               |
| -        | Memory     | memory manipulation                          |
|          |            |                                              |
| Hardware | Display    | display interface                            |
| -        | Background | tiled backgrounds                            |
| -        | Sprite     | sprites                                      |
| -        | Window     | display windows (e.g. viewports)             |
| -        | Audio      | digital audio driver                         |
| -        | Timer      | hardware timers                              |
| -        | DMA        | direct memory access                         |
| -        | Input      | input detection                              |
| -        | Interrupt  | interrupt handling                           |
| -        | System     | system control                               |
| -        | Cartridge  | game cartridge                               |
| -        | Backup     | backup memory                                |
|          |            |                                              |
| Debug    | Profiler   | code profiler                                |
| -        | mGBA       | printing to mGBA output (e.g. mgba_printf)   |

Most modules are accompanied by a manual: see the [doc](doc) directory.
An overview of the library and a brief description of each module can be
found in the [libsimplegba](doc/libsimplegba.adoc) manual.

## Building
First, make sure the *arm-none-eabi-gcc* compiler and *make* tool are
installed. Then, run `make` to generate the static library file
`bin/libsimplegba.a`.

## Usage
Follow these steps to use the library:
- Obtain *libsimplegba* (for example, by adding it as a git submodule).
- Add the `include` directory to the include path.
- Add the `bin` directory to the library search path.
- Add `libsimplegba` to the list of libraries to link against.
- Add the `-nostartfiles` and `-T<lib-path>/lnkscript` linker flags.

### Makefile example
Assuming the library is located at `lib/libsimplegba`, add these lines
to `Makefile`:

```Makefile
# linker script
LDFLAGS += -nostartfiles -Tlib/libsimplegba/lnkscript

# libsimplegba
CPPFLAGS += -Ilib/libsimplegba/include
LDFLAGS  += -Llib/libsimplegba/bin
LDLIBS   += -lsimplegba
```

## License
*libsimplegba* is released under the GNU General Public License, either
version 3 of the License or any later version. Any work that uses this
library must be released under a [GPL-Compatible Free Software
license](https://www.gnu.org/licenses/license-list.html).
