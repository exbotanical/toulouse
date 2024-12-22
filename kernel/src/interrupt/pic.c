#include "interrupt/pic.h"

#include "arch/x86.h"

void
pic_init (void) {
  // Remap interrupts for PIC1 (master)
  outb(PIC_MASTER, ICW1_RESET);
  // ICW2 (sets the base interrupt vector address)
  outb(PIC_MASTER_DATA, IRQ0_ADDR);
  // ICW3 specifies the cascading relationship
  // (e.g. which IRQ line connects the master and slave PICs, usually CASCADE_IRQ).
  outb(PIC_MASTER_DATA, 1 << CASCADE_IRQ);
  outb(PIC_MASTER_DATA, ICW4_8086EOI);

  // Remap interrupts for PIC2 (slave)
  outb(PIC_SLAVE, ICW1_RESET);
  outb(PIC_SLAVE_DATA, IRQ8_ADDR);
  outb(PIC_SLAVE_DATA, CASCADE_IRQ);
  outb(PIC_SLAVE_DATA, ICW4_8086EOI);

  // Mask all IRQs except the cascade (left unmasked because it's used by the slave PIC to signal
  // the master about interrupts from IRQs 8 - 15)
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
  outb(PIC_MASTER, ocw3);
  outb(PIC_SLAVE, ocw3);
  return (inb(PIC_SLAVE) << 8) | inb(PIC_MASTER);
}
