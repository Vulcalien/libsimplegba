# libsimplegba
*libsimplegba* is a collection of functions and constants that can be
used to interface with the GBA hardware hiding some of the complexity
and low-level details.

The library focuses on ease of use without sacrificing efficiency: most
of the functions are implemented as *inline functions*, meaning that
under most circumstances they are just as efficient as writing to the
hardware registers directly.

## Modules
The following modules are available:
| Category  | Module     | Description                                 |
| :-------: | ---------- | ------------------------------------------- |
| Misc      | Base       | basic definitions (types, macros...)        |
| -         | Math       | various mathematical functions              |
| -         | Random     | pseudo-random number generator              |
| -         | Memory     | memory manipulation                         |
|           |            |                                             |
| Hardware  | Display    | display interface                           |
| -         | Background | tiled backgrounds                           |
| -         | Sprite     | sprites                                     |
| -         | Window     | display windows (e.g. viewports)            |
| -         | Audio      | digital audio driver                        |
| -         | Timer      | hardware timers                             |
| -         | DMA        | direct memory access                        |
| -         | Input      | input detection                             |
| -         | Interrupt  | interrupt handling                          |
| -         | System     | system control                              |
|           |            |                                             |
| Cartridge | Cartridge  | game cartridge (e.g. header)                |
| -         | Backup     | backup memory                               |
| -         | GPIO       | 4-bit GPIO port                             |
|           |            |                                             |
| Debug     | Profiler   | code profiler                               |
| -         | mGBA       | printing to mGBA output (e.g. mgba_printf)  |

Most modules are accompanied by a manual: see the [doc](doc) directory.

## Building
First, make sure the *arm-none-eabi-gcc* compiler and *make* tool are
installed. Then, run `make` to generate the static library file
`bin/libsimplegba.a`.

## Usage
Follow these instructions to use *libsimplegba* in a project.

### Adding the library
To let the build tool know where to find library files:
- Add the `include` directory to the include path
- Add the `bin` directory to the library search path
- Add `libsimplegba` to the list of libraries to link against
- Set `lnkscript` as the linker script

If using a `Makefile` to build the program and assuming the library is
located at `lib/libsimplegba`, add these lines:

```Makefile
# linker script
LDFLAGS += -nostartfiles -Tlib/libsimplegba/lnkscript

# libsimplegba
CPPFLAGS += -Ilib/libsimplegba/include
LDFLAGS  += -Llib/libsimplegba/bin
LDLIBS   += -lsimplegba
```

### Using the library
Use `#include <libsimplegba.h>` to retrieve the library's single header.
All library functions, types, constants... will be available.

The library expects the `void AgbMain(void)` function to be defined,
which will be used as the entry point of the program. This function
should first perform initialization, then enter an infinite loop. For
example:

```c
void AgbMain(void) {
    interrupt_toggle(IRQ_VBLANK, true);

    input_init(30, 2);
    audio_init(AUDIO_MIXER);
    backup_init(BACKUP_SRAM);

    // init...
    while(true) {
        audio_update();
        input_update();

        // tick...
        interrupt_wait(IRQ_VBLANK);
        // draw...
    }
}
```

## License
*libsimplegba* is released under the GNU General Public License, either
version 3 of the License or any later version. Any work that uses this
library must be released under a [GPL-Compatible Free Software
license](https://www.gnu.org/licenses/license-list.html).
