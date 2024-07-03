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
    Sound               <gba/sound.h>
    Timer               <gba/timer.h>
    DMA                 <gba/dma.h>
    Input               <gba/input.h>
    Interrupt           <gba/interrupt.h>

Debug
    Profiler            <debug/profiler.h>
    mGBA                <debug/mgba.h>
```

Most modules are accompanied by a manual: see the [doc](doc) directory.
An overview of the library and a brief description of each module can be
found in the [libsimplegba](doc/libsimplegba.adoc) manual.

## Building
To build the library, run `make`. This will generate the static library
file `bin/libsimplegba.a`.

## Using libsimplegba
### CPPFLAGS
To gain access to the library's header files, add the `include`
directory to the include search path (`-I<lib-path>/include`).

### LDFLAGS and LDLIBS
To statically link against `bin/libsimplegba.a`, add the `bin` directory
to the library search path (`-L<lib-path>/bin`) and the `libsimplegba.a`
file to the list of libraries (`-lsimplegba`).

To use the linker script provided by this library, add these linker
flags: `-nostartfiles` and `-T<lib-path>/lnkscript`.

### Example
Assuming the library is located at `lib/libsimplegba`:

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
