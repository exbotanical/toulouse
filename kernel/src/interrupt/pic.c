#include "interrupt/pic.h"

#include "arch/x86.h"

void
pic_init (void) {
  // The PIC has a fixed set of interrupt request (IRQ) lines, and these lines are used by various
  // hardware devices. When two or more devices try to generate interrupts at the same time, the PIC
  // must prioritize which one is handled first.

  // Why must we configure the PIC?
  // 1) Dynamic Interrupt Assignment: Different operating systems (or even different components of
  // the same OS) might use different interrupt lines for various devices.
  // 2) Interrupt Priorities: The PIC allows the OS to set priorities for interrupt handling.
  // 3) Hardware detection: When the OS is first booted, it doesn't necessarily know exactly how the
  // hardware is configured.
  // 4) Customizability

  // Most x86 systems have two 8259 PIC chips: one is the master PIC, the other the
  // slave PIC. The slave is connected to the master via a cascade aka IRQ2. Each PIC can handle 8
  // IRQ lines, so we have a combined total of 16 IRQs (0 - 15).

  // Send ICW1 to master PIC. Tells it: "We're starting initialization."
  outb(PIC_MASTER, ICW1_RESET);

  // ICW2 (sets the base interrupt vector address).
  outb(PIC_MASTER_DATA, IRQ0_ADDR);
  // ICW3 specifies the cascading relationship
  // (e.g. which IRQ line connects the master and slave PICs, usually CASCADE_IRQ).
  outb(PIC_MASTER_DATA, 1 << CASCADE_IRQ);
  outb(PIC_MASTER_DATA, ICW4_8086EOI);

  // Remap interrupts for PIC2 (slave)
  outb(PIC_SLAVE, ICW1_RESET);
  outb(PIC_SLAVE_DATA, IRQ8_ADDR);
  // Tell slave which IRQ line it's connected to on master.
  outb(PIC_SLAVE_DATA, CASCADE_IRQ);
  outb(PIC_SLAVE_DATA, ICW4_8086EOI);

  // Mask all IRQs except the cascade (left unmasked because it's used by the slave PIC to signal
  // the master about interrupts from IRQs 8 - 15).
  // In other words, this ensures the master will forward interrupts only from the slave.
  outb(PIC_MASTER_DATA, ~(1 << CASCADE_IRQ));
  outb(PIC_SLAVE_DATA, OCW1);
}

void
pic_irq_ack (int irq_num) {
  if (irq_num > 7) {
    outb(PIC_SLAVE, EOI);
  }
  outb(PIC_MASTER, EOI);
}

unsigned short int
pic_get_irq_register (int ocw3) {
  // Ask the master PIC to prepare the requested register (IRR/ISR).
  outb(PIC_MASTER, ocw3);
  // Ask the slave PIC to do the same.
  outb(PIC_SLAVE, ocw3);
  // Read the slave's 8 bits (IRQs 8–15) and the master's 8 bits (IRQs 0–7).
  // The slave's value is shifted up into the high byte (<< 8) because slave IRQs are IRQ8–IRQ15,
  // while master handles IRQ0–IRQ7.
  // The result is a 16-bit number with:
  // * High byte = IRQs 8–15 (slave)
  // * Low byte = IRQs 0–7 (master)
  return (inb(PIC_SLAVE) << 8) | inb(PIC_MASTER);
}
