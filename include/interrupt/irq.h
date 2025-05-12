#ifndef INTERRUPT_IRQ_H
#define INTERRUPT_IRQ_H

#include "interrupt/signal.h"
#include "lib/types.h"

#define NUM_IRQS      16
#define IRQ_BH_ACTIVE 0x01

/**
 * Represents the config for an interrupt handler
 */
typedef struct interrupt interrupt_t;

struct interrupt {
  /**
   * The number of times the interrupt handler has been invoked. Used for debugging.
   */
  unsigned int ticks;
  /**
   * Human-readable name for the interrupt. Used for debugging.
   */
  char        *name;
  /**
   * The actual interrupt handler.
   */
  void (*handler)(int, sig_context_t *);
  /**
   * Linked list pointer to the next node.
   */
  interrupt_t *next;
};

/**
 * Represents the config for a bottom half handler.
 */
typedef struct interrupt_bh interrupt_bh_t;

struct interrupt_bh {
  /**
   * Holds flags such as whether the handler should be executed.
   */
  int flags;
  /**
   * The actual bottom half handler.
   */
  void (*fn)(sig_context_t *);
  /**
   * Linked list pointer to the next node.
   */
  interrupt_bh_t *next;
};

/**
 * Table of IRQ handlers
 */
extern interrupt_t *irq_table[NUM_IRQS];

extern void exception_0(void);
extern void exception_1(void);
extern void exception_2(void);
extern void exception_3(void);
extern void exception_4(void);
extern void exception_5(void);
extern void exception_6(void);
extern void exception_7(void);
extern void exception_8(void);
extern void exception_9(void);
extern void exception_10(void);
extern void exception_11(void);
extern void exception_12(void);
extern void exception_13(void);
extern void exception_14(void);
extern void exception_15(void);
extern void exception_16(void);
extern void exception_17(void);
extern void exception_18(void);
extern void exception_19(void);
extern void exception_20(void);
extern void exception_21(void);
extern void exception_22(void);
extern void exception_23(void);
extern void exception_24(void);
extern void exception_25(void);
extern void exception_26(void);
extern void exception_27(void);
extern void exception_28(void);
extern void exception_29(void);
extern void exception_30(void);
extern void exception_31(void);

extern void irq_0(void);
extern void irq_1(void);
extern void irq_2(void);
extern void irq_3(void);
extern void irq_4(void);
extern void irq_5(void);
extern void irq_6(void);
extern void irq_7(void);
extern void irq_8(void);
extern void irq_9(void);
extern void irq_10(void);
extern void irq_11(void);
extern void irq_12(void);
extern void irq_13(void);
extern void irq_14(void);
extern void irq_15(void);
extern void irq_unknown(void);

/**
 * Sets up the IRQ table.
 */
void irq_init(void);

/**
 * Adds a new bottom half handler to the list that is iterated on every IRQ.
 * Only handlers with the `IRQ_BH_ACTIVE` flag set are invoked.
 *
 * @param new
 *
 */
void irq_bottom_half_register(interrupt_bh_t *new);

/**
 * Invokes the bottom half handler.
 *
 * @param sc context about the signal that triggered the interrupt
 */
void irq_bottom_half_exec(sig_context_t *sc);

/**
 * Unmasks (enables) a specific IRQ line on the PIC, allowing it
 * to generate interrupts.
 *
 * @param irq_num The IRQ line to enable
 */
void irq_enable(int irq_num);

/**
 * Masks (disables) a specific IRQ line on the PIC, preventing it
 * from generating interrupts.
 *
 * @param irq_num The IRQ line to disable
 */
void irq_disable(int irq_num);

/**
 * Adds a new interrupt handler to the irq_table.
 *
 * @param irq_num The number of the IRQ
 * @param interrupt A pointer to the interrupt handler config
 * @return RET_OK if registering the IRQ was successful
 * @return RET_FAIL if registering the IRQ was not successful, usually because the irq_num was
 * invalid, or the IRQ is already registered
 */
retval_t irq_register(int irq_num, interrupt_t *interrupt);

/**
 * Wrapper for all interrupt handlers. Performs spurious interrupt checks and sends an EOI to the
 * PIC. Guarantees no reentrant interrupts.
 *
 * @param irq_num
 * @param sc
 */
void irq_handler(int irq_num, sig_context_t sc);

/**
 * Handler for unknown interrupts!
 */
void irq_unknown_handler(void);

/**
 * Dedicated spurious interrupt handler.
 *
 * @param irq_num
 */
void irq_spurious_interrupt_handler(int irq_num);

#endif /* INTERRUPT_IRQ_H */
