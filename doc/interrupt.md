# Interrupt driver

```c
#include "interrupt.h"

void interrupt_init(void);

void interrupt_enable(u8 irq);
void interrupt_disable(u8 irq);

void interrupt_set_isr(u8 irq, void (*isr)(void));
```

| IRQ identifiers |             |            |            |
| --------------- | ----------- | ---------- | ---------- |
| IRQ_VBLANK      | IRQ_HBLANK  | IRQ_VCOUNT |            |
| IRQ_TIMER0      | IRQ_TIMER1  | IRQ_TIMER2 | IRQ_TIMER3 |
| IRQ_DMA0        | IRQ_DMA1    | IRQ_DMA2   | IRQ_DMA3   |
| IRQ_KEYPAD      | IRQ_GAMEPAK | IRQ_SERIAL |            |

## Usage

### interrupt_init

`interrupt_init` initializes the driver. This only needs to be called
once, and it should be done before initializing anything else that uses
interrupts (such as the sound driver).

### interrupt_enable, interrupt_disable

`interrupt_enable` enables the given interrupt (IRQ), while
`interrupt_disable` disables it. One of the IRQ identifiers can be
passed as argument.

### interrupt_set_isr

`interrupt_set_isr` maps an IRQ to its *ISR* (Interrupt Service
Routine). It takes one of the interrupt identifiers (listed above) and a
function pointer.

If the specified function pointer is NULL, the driver will perform no
action when that interrupt is detected.

Note that each interrupt can be mapped to just one ISR at a time. If
multiple ISRs are required, the recommended approach is to write a
function that calls all the others:
```c
void foo_isr(void) {
    foo_isr_a();
    foo_isr_b();
    ...
}
```

## Examples

Enable the VBLANK interrupt and map an ISR to it:
```c
int AgbMain(void) {
    interrupt_init();

    interrupt_enable(IRQ_VBLANK);
    interrupt_set_isr(IRQ_VBLANK, vblank_isr);

    ...
}
```
