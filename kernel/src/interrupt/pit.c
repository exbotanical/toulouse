#include "interrupt/pit.h"

#include "arch/x86.h"

void
pit_init (unsigned short int hz) {
  outb(
    // The port address of the PIT mode register. We'll send a control word here.
    MODEREG,
    // PIT has three channels but 0 is commonly used for sys timer interrupts.
    SEL_CHAN0
      // We're telling the PIT to expect the divisor to be sent in two parts -
      // LSB first, then MSB
      | LSB_MSB
      // Set the mode to "rate generator" i.e. generate periodic interrupts
      | RATE_GEN
      // Counter to operate in binary mode
      | BINARY_CTR
  );
  // We divide the input clock (OSCIL) by a divisor to get the desired frequency (hertz here).
  // This is what the PIT will use when generating interrupts.
  outb(CHANNEL0, (OSCIL / hz) & 0xFF);  // lsb
  outb(CHANNEL0, (OSCIL / hz) >> 8);    // msb
}
