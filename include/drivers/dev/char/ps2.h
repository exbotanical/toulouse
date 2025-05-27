#ifndef DRIVERS_DEV_CHAR_PS2_H
#define DRIVERS_DEV_CHAR_PS2_H
// clang-format off

// PS/2 is a legacy interface standard for connecting input devices like keyboards and mice to a PC.
// It's handled through a PS/2 controller (often integrated into the system’s chipset) which
// connects via two ports:
// * Channel 1: Usually the keyboard.
// * Channel 2: Usually the mouse (if present).
//
// Other notes:
// * Translation refers to scan code (number sent by a keyboard) translation done by the PS/2
//   controller. Basically, different devices send different scan code sets and the PS/2 can translate them.
// * typematic rate is how frequently a computer will repeat a single character when a key is held down continuously

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
// Note: This self-test is actually testing the hardware itself, whereas the ch1/ch2 tests are for the individual device channels/interfaces
#define PS2_CMD_SELF_TEST   0xAA /* Self-test command */
#define PS2_CMD_GET_IFACE   0xCA /* Get the current interface */

/* Device Commands */
#define PS2_KB_SETLED       0xED /* Set/reset status indicators (LEDs) */
#define PS2_KB_GETSETSCAN   0xF0 /* Keyboard get/set scan code */
#define PS2_DEV_IDENTIFY    0xF2 /* Identify device (for PS/2 only) */
#define PS2_DEV_RATE        0xF3 /* Set typematic rate/delay */
#define PS2_DEV_ENABLE      0xF4 /* Enable keyboard scanning */
#define PS2_KB_DISABLE      0xF5 /* Disable keyboard scanning */
#define PS2_DEV_RESET       0xFF /* Device reset */

/* Misc */
#define DEV_RESET_OK        0xAA /* Self-test passed */
#define DEV_ACK             0xFA /* acknowledge */

#define PS2_TIMEOUT         500000
// clang-format on

extern volatile unsigned char ack;

/**
 * Waits for the device to send back an ACK.
 * Returns true if we got an ACK during the timeout period.
 *
 * See: https://wiki.osdev.org/I8042_PS/2_Controller
 */
bool ps2_await_ack(void);

/**
 * Purges the PS/2 input buffer
 */
void ps2_clear_buffer(void);

/**
 * Reads from the PS/2 controller
 */
unsigned char ps2_read(const unsigned char port);

/**
 * Writes to the PS/2 controller
 */
void ps2_write(const unsigned char port, const unsigned char byte);

/**
 * Configure the PS/2 device and enable keyboard support
 */
void ps2_init(void);

#endif /* DRIVERS_DEV_CHAR_PS2_H */
