# libsimplegba
*libsimplegba* is a utility library for the *Game Boy Advance* that
provides interfaces to hardware components, debug tools, compilation
files and more.

## Modules
The library is made up of many *modules*, each handling a specific area
or hardware component. Most modules are accompanied by a manual: see the
[doc](doc) directory.

The following modules are available:
| Category  | Module     | Description                                 |
| :-------: | ---------- | ------------------------------------------- |
| Misc      | Types      | basic data types                            |
| -         | Attributes | compilation attributes                      |
| -         | Math       | various mathematical functions              |
| -         | Random     | pseudo-random number generator              |
| -         | Memory     | memory manipulation                         |
|           |            |                                             |
| Hardware  | Display    | display interface                           |
| -         | Background | tiled backgrounds                           |
| -         | Sprite     | sprites                                     |
| -         | Window     | display windows (e.g. viewports)            |
| -         | Effects    | color effects (e.g. blending)               |
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
| -         | RTC        | real-time clock                             |
|           |            |                                             |
| Debug     | Profiler   | code profiler                               |
| -         | mGBA       | printing to mGBA output (e.g. mgba_printf)  |

## Usage
### Building the library
Make sure the *arm-none-eabi-gcc* compiler and the *GNU make* tool.
Then, simply run `make` to generate the static library file
`bin/libsimplegba.a`.

### Adding the library
Assuming the library is located at `lib/libsimplegba`, instruct your
build tool as follows.

```Makefile
# linker script
LDFLAGS += -nostartfiles -Tlib/libsimplegba/files/gba.ld

# libsimplegba
CPPFLAGS += -Ilib/libsimplegba/include
LDFLAGS  += -Llib/libsimplegba/bin
LDLIBS   += -lsimplegba
```

### Defining the ROM header
*Game Boy Advance* ROMs need a header, which contains metadata for the
program. Copy the template from `files/header.s` into your source code
directory, then change *Game Title*, *Game Code* and (optionally)
*Software Version*.

After making those changes, you'll need to update the *Header Checksum*.
To do that, compile the ROM and run the `tools/verify-rom` script on it.
Use the checksum the script prints, then verify the ROM again.

### Using the library
Use `#include <libsimplegba.h>` to retrieve the library's single header.
All the functions, types, constants... will be available.

Your program's entry point will be the `AgbMain` function. This function
should perform initialization, then enter an infinite loop. For example:

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
