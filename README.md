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
*libsimplegba* is composed of various modules, each specializing in a
specific hardware component or task.

Here is the list of modules available (and their corresponding headers):
```
Misc
    Base                <base.h>
    Math                <math.h>
    Random              <random.h>
    Memory              <memory.h>

GBA Hardware
    Display             <gba/display.h>
    Background          <gba/background.h>
    Sprite              <gba/sprite.h>
    Window              <gba/window.h>
    Audio               <gba/audio.h>
    Timer               <gba/timer.h>
    DMA                 <gba/dma.h>
    Input               <gba/input.h>
    Interrupt           <gba/interrupt.h>
    System              <gba/system.h>
    Cartridge           <gba/cartridge.h>
    Backup              <gba/backup.h>

Debug
    Profiler            <debug/profiler.h>
    mGBA                <debug/mgba.h>
```

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
