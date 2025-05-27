#include "interrupt/irq.h"

#include "arch/eflags.h"
#include "arch/interrupt.h"  // TODO: Move
#include "arch/x86.h"
#include "drivers/dev/char/tmpcon.h"
#include "interrupt/pic.h"
#include "lib/string.h"

interrupt_t *irq_table[NUM_IRQS];

/**
 * Bottom half handlers table.
 *
 * Bottom half execution is a technique for deferring non-urgent work until after
 * the immediate interrupt handling is done.
 * Effectively, you have an interrupt handler ("top half") that runs quickly, and any heavier work
 * (like copying data, printing to screen, etc.) is deferred to a "bottom half", which runs after
 * the interrupt is handled, when it's safe to do more.
 *
 * This is not 1:1 with interrupts. Bottom half functions are shared resources — a global list of
 * deferred handlers, and it's up to each handler to know whether it should do something or not.
 *
 * It's up to the top half IRQ handler to mark the bottom half handler active. This looks something
 * like:
 *
 * `some_bh->flags |= IRQ_BH_ACTIVE;`
 *
 * From Linux Device Drivers, Second Edition:
 *
 * "The big difference between the top-half handler and the bottom half is that all interrupts are
 * enabled during execution of the bottom half—that's why it runs at a safer time. In the typical
 * scenario, the top half saves device data to a device-specific buffer, schedules its bottom half,
 * and exits: this is very fast. The bottom half then performs whatever other work is required, such
 * as awakening processes, starting up another I/O operation, and so on. This setup permits the top
 * half to service a new interrupt while the bottom half is still working."
 */
static interrupt_bh_t *bh_table = NULL;

void
irq_init (void) {
  kmemset(irq_table, 0, sizeof(irq_table));
}

void
irq_bottom_half_register (interrupt_bh_t *new) {
  unsigned int flags = eflags_get();
  int_disable();

  interrupt_bh_t **b = &bh_table;
  while (*b) {
    b = &(*b)->next;
  }
  *b = new;

  eflags_set(flags);
}

void
irq_bottom_half_exec (sig_context_t *sc) {
  interrupt_bh_t *bh;

  bh = bh_table;
  while (bh) {
    if (bh->flags & IRQ_BH_ACTIVE) {
      bh->flags &= ~IRQ_BH_ACTIVE;
      bh->fn(sc);
    }
    bh = bh->next;
  }
}

void
irq_enable (int irq_num) {
  // Determine whether the IRQ is from the master or the slave PIC
  int addr  = (irq_num > 7) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;

  // Mask the IRQ number to 0–7. Each PIC has only 8 lines (0–7), so we need to reduce it to local
  // numbering. For example, dealing with IRQ 13, which belongs to the slave PIC:
  // 13 & 0x07 → 00001101 & 00000111 → 00000101 → 5
  // i.e. on the slave PIC, IRQ 13 maps to line 5.
  irq_num  &= 0x0007;

  // Update the PIC's Interrupt Mask Register (IMR).
  // Breakdown:
  // inb(addr) → read the current IRQ mask
  // 1 << irq_num → bit corresponding to the IRQ we're enabling
  // AND-ing w/ ~(1 << irq_num) clears that bit, which unmasks the IRQ
  outb(addr, inb(addr) & ~(1 << irq_num));
}

void
irq_disable (int irq_num) {
  int addr  = (irq_num > 7) ? PIC_SLAVE_DATA : PIC_MASTER_DATA;
  irq_num  &= 0x0007;
  // OR-ing with (1 << irq_num) sets that bit to 1, which masks (disables) the IRQ line
  outb(addr, inb(addr) | (1 << irq_num));
}

retval_t
irq_register (int irq_num, interrupt_t *new_irq) {
  if (irq_num < 0 || irq_num >= NUM_IRQS) {
    klogf_warn("%s(): interrupt %d is greater than NUM_IRQS (%d)\n", __func__, irq_num, NUM_IRQS);
    return RET_FAIL;
  }

  interrupt_t **irq = &irq_table[irq_num];
  while (*irq) {
    if (*irq == new_irq) {
      klogf_warn("%s(): interrupt %d already registered\n", __func__, irq_num);
      return RET_FAIL;
    }
    irq = &(*irq)->next;
  }
  *irq           = new_irq;

  new_irq->ticks = 0;

  return RET_OK;
}

void
irq_handler (int irq_num, sig_context_t sc) {
  interrupt_t *irq;
  // Temporarily prevent reentrant interrupts while we're handling this one
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
  kprintf("Unknown IRQ received.\n");
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
    // "For a spurious IRQ, there is no real IRQ and the PIC chip's ISR (In Service Register) flag
    // for the corresponding IRQ will not be set. This means that the interrupt handler must not
    // send an EOI back to the PIC to reset the ISR flag."
    return;
  }

  pic_irq_ack(irq_num);
}
