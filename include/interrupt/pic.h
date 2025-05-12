#ifndef INTERRUPT_PIC_H
#define INTERRUPT_PIC_H

// interrupt vector base addresses
#define IRQ0_ADDR            0x20
#define IRQ8_ADDR            0x28

#define PIC_DATA_PORT_OFFSET 0x01
#define PIC_MASTER           0x20
#define PIC_SLAVE            0xA0
#define PIC_MASTER_DATA      PIC_MASTER + PIC_DATA_PORT_OFFSET
#define PIC_SLAVE_DATA       PIC_SLAVE + PIC_DATA_PORT_OFFSET

/* Initialization Commands */

/**
 * ICW1_INIT and ICW1_ICW4
 * ICW1_INIT -> Indicates that the PIC is being initialized.
 * ICW1_ICW4 -> Specifies that ICW4 (Initialization Command Word 4) will be sent during
 * initialization.
 */
#define ICW1_RESET           0x11
/**
 * This is the IRQ line on the master PIC where the slave PIC is connected.
 */
#define CASCADE_IRQ          0x02
/**
 * Configures the PIC for operation in 8086/88 mode and enables End of Interrupt (EOI) mode.
 */
#define ICW4_8086EOI         0x01
/**
 * IRQ ready
 */
#define PIC_READ_IRR         0x0A
/**
 * IRQ service
 */
#define PIC_READ_ISR         0x0B

/* Operational Commands */

/**
 * Disable all IRQs
 */
#define OCW1                 0xFF
/**
 * End of interrupt
 */
#define EOI                  0x20

/**
 * Remaps all interrupts and masks all IRQs (except for the cascade because we need the slave to
 * process IRQs 8 - 15).
 */
void pic_init(void);

/**
 * Acknowledges the IRQ on the appropriate PIC (master or slave) by sending an End-of-Interrupt
 * (EOI) to the PIC to let it know we're done with this interrupt, and it can start accepting new
 * ones.
 * @param irq_num
 */
void pic_irq_ack(int irq_num);

/**
 * Sends the OCW3 command to retrieve the register values;
 * reads the current IRQ status from both the master and slave PICs. Combines the results from both
 * PICs into a single 16-bit value that indicates which IRQs are active.
 */
unsigned short int pic_get_irq_register(int ocw3);

#endif /* INTERRUPT_PIC_H */
