#ifndef DRIVERS_DEV_CHAR_PS2_H
#define DRIVERS_DEV_CHAR_PS2_H

// PS/2 is a legacy interface standard for connecting input devices like keyboards and mice to a PC.
// It's handled through a PS/2 controller (often integrated into the system’s chipset) which
// connects via two ports:
// * Channel 1: Usually the keyboard.
// * Channel 2: Usually the mouse (if present).

// Other notes:
// * Translation refers to scan code (number sent by a keyboard) translation done by the PS/2
// controller. Basically, different devices send different scan code sets and the PS/2 can translate
// them.

#define PS2_DATA_PORT       0x60 /* I/O data port */
#define PS2_COMMAND_PORT    0x64 /* Command/control port */
#define PS2_STATUS_PORT     0x64 /* Status register port */

/* Status Register */
#define PS2_STAT_OUTBUSY    0x01 /* Output buffer full, don't read yet */
#define PS2_STAT_INBUSY     0x02 /* Input buffer full, don't write yet */

/* Controller Commands */
#define PS2_CMD_RECV_CONFIG 0x20 /* Read controller's config byte */
#define PS2_CMD_SEND_CONFIG 0x60 /* Write controller's config byte */

#define PS2_CMD_ENABLE_CH1  0xAE /* Enable channel 1 */
#define PS2_CMD_DISABLE_CH1 0xAD /* Disable channel 1 */
#define PS2_CMD_TEST_CH1    0xAB /* Test interface for channel 1 */

#define PS2_CMD_ENABLE_CH2  0xA8 /* Enable channel 2 (if any) */
#define PS2_CMD_DISABLE_CH2 0xA7 /* Disable channel 2 (if any) */
#define PS2_CMD_TEST_CH2    0xA9 /* Test interface for channel 2 */

/* Self-test command. Note: This self-test is actually testing the hardware itself, whereas the
 * ch1/ch2 tests are for the individual device channels/interdfaces */
#define PS2_CMD_SELF_TEST   0xAA
#define PS2_CMD_GET_IFACE   0xCA /* Get the current interface */

#define PS2_TIMEOUT         500000

extern volatile unsigned char ack;

/**
 * Configure the PS/2 device and enable keyboard support
 */
void ps2_init(void);

#endif /* DRIVERS_DEV_CHAR_PS2_H */
