#include "kernel.h"

#include "arch/cpu.h"
#include "arch/interrupt.h"
#include "drivers/dev/keyboard.h"
#include "interrupt/irq.h"

void noreturn
kernel_stop (void) {
  int_disable();

  for (unsigned int n = 0; n < NUM_IRQS; n++) {
    irq_disable(n);
  }
  irq_enable(KEYBOARD_IRQ);

  int_enable();

  cpu_idle();
}
