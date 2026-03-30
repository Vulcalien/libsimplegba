#pragma once

#include "libsimplegba/base.h"

// Misc
#include "libsimplegba/misc/types.h"
#include "libsimplegba/misc/math.h"
#include "libsimplegba/misc/random.h"
#include "libsimplegba/misc/memory.h"

// Hardware
#include "libsimplegba/hw/display.h"
#include "libsimplegba/hw/background.h"
#include "libsimplegba/hw/sprite.h"
#include "libsimplegba/hw/window.h"
#include "libsimplegba/hw/effects.h"
#include "libsimplegba/hw/audio.h"
#include "libsimplegba/hw/timer.h"
#include "libsimplegba/hw/dma.h"
#include "libsimplegba/hw/input.h"
#include "libsimplegba/hw/interrupt.h"
#include "libsimplegba/hw/system.h"

// Cartridge
#include "libsimplegba/cart/cartridge.h"
#include "libsimplegba/cart/backup.h"
#include "libsimplegba/cart/gpio.h"
#include "libsimplegba/cart/rtc.h"

// Debug
#include "libsimplegba/debug/profiler.h"
#include "libsimplegba/debug/mgba.h"
