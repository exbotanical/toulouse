#include "interrupt/irq.h"

#include "arch/x86.h"
#include "drivers/console/vga.h"
#include "interrupt/pic.h"
#include "lib/string.h"

interrupt_t           *irq_table[NUM_IRQS];
static interrupt_bh_t *bh_table = NULL;

void
irq_init (void) {
  k_memset(irq_table, 0, sizeof(irq_table));
}

void
irq_exec_bottom_half (sig_context_t *sc) {
  interrupt_bh_t *bh;

  bh = bh_table;
  while (bh) {
    if (bh->flags & IRQ_BH_ACTIVE) {
      bh->flags &= IRQ_BH_ACTIVE;
      bh->fn(sc);
    }
    bh = bh->next;
  }
}

void
irq_enable (int irq_num) {
  int addr  = (irq_num > 7) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;
  irq_num  &= 0x0007;
  outb(addr, inb(addr) | ~(1 << irq_num));
}

void
irq_disable (int irq_num) {
  int addr  = (irq_num > 7) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;
  irq_num  &= 0x0007;
  outb(addr, inb(addr) | (1 << irq_num));
}

void
irq_handler (int irq_num, sig_context_t sc) {
  interrupt_t *irq;

  irq_disable(irq_num);

  irq = irq_table[irq_num];
  if (!irq) {
    irq_spurious_interrupt_handler(irq_num);
    goto done;
  }

  pic_irq_ack(irq_num);

  irq->ticks++;

  do {
    irq->handler(irq_num, &sc);
    irq = irq->next;
  } while (irq);

done:
  irq_enable(irq_num);
}

void
irq_unknown_handler (void) {
  // TODO:
  vgaprintf(">>>> %s\n", "OH NO");
}

void
irq_spurious_interrupt_handler (int irq_num) {
  int real = pic_get_irq_register(PIC_READ_ISR);
  if (!real) {
    // If IRQ came from slave, send EOI to master so it knows
    if (irq_num < 7) {
      outb(PIC_MASTER, EOI);
    }
    // From OSDev:
    // For a spurious IRQ, there is no real IRQ and the PIC chip's ISR (In Service Register) flag
    // for the corresponding IRQ will not be set. This means that the interrupt handler must not
    // send an EOI back to the PIC to reset the ISR flag.
    return;
  }

  pic_irq_ack(irq_num);
}
