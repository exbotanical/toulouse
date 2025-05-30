#include "drivers/dev/char/ps2.h"

#include "arch/x86.h"

extern volatile unsigned char ack;

static void
ps2_delay (void) {
  for (int i = 0; i < 1000; i++) {
    idle();
  }
}

/**
 * Waits for the controller's output buffer to be clear, or for controller acknowledgement
 */
static bool
is_ready_to_read (void) {
  for (int n = 0; n < PS2_TIMEOUT; n++) {
    if (ack) {
      return true;
    }
    if (inb(PS2_STATUS_PORT) & PS2_STAT_OUTBUSY) {
      return true;
    }
  }
  return false;
}

/**
 * Waits for the controller's input buffer to be clear
 */
static bool
is_ready_to_write (void) {
  for (int n = 0; n < PS2_TIMEOUT; n++) {
    if (!(inb(PS2_STATUS_PORT) & PS2_STAT_INBUSY)) {
      return true;
    }
  }
  return false;
}

void
ps2_set_leds (unsigned char led_status) {
  ps2_write(PS2_DATA_PORT, PS2_KB_SETLED);
  ps2_await_ack();

  ps2_write(PS2_DATA_PORT, led_status);
  ps2_await_ack();
}

bool
ps2_await_ack (void) {
  if (is_ready_to_read()) {
    for (unsigned int i = 0; i < 1000; i++) {
      if (inb(PS2_DATA_PORT) == DEV_ACK) {
        return true;
      }
      ps2_delay();
    }
  }
  return false;
}

unsigned char
ps2_read (const unsigned char port) {
  if (is_ready_to_read()) {
    return inb(port);
  }

  return 0;
}

void
ps2_write (const unsigned char port, const unsigned char byte) {
  ack = 0;

  if (is_ready_to_write()) {
    outb(port, byte);
  }
}

void
ps2_clear_buffer (void) {
  for (int i = 0; i < 1000; i++) {
    ps2_delay();
    if (inb(PS2_STATUS_PORT) & PS2_STAT_OUTBUSY) {
      // We're basically emptying it by reading out of it
      ps2_read(PS2_DATA_PORT);
      continue;
    }
    break;
  }
}

void
ps2_init (void) {
  int           errno;
  char          type            = 0;
  unsigned int  supported_ports = 0;
  unsigned char config, config2;
  config = config2 = 0;

  // Disable devices before we do some reconfiguration
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_DISABLE_CH1);
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_DISABLE_CH2);

  // Flush the buffer to clear any leftover input
  inb(PS2_DATA_PORT);

  // Ask the controller for its current configuration byte and store it
  // We need to know the current state (e.g. IRQs enabled, translation, etc) so we can safely modify
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_RECV_CONFIG);
  config = ps2_read(PS2_DATA_PORT);

  // Disable both IRQs (0x01 and 0x02), and turn off translation (0x40).
  // Interrupts and translation must be disabled temporarily to avoid issues during setup.
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config & ~(0x01 | 0x02 | 0x40));

  // Check the controller's built-in self-test...
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SELF_TEST);
  // ...then check if the result is 0x55, i.e. "OK".
  if ((errno = ps2_read(PS2_DATA_PORT)) != 0x55) {
    // There's no point continuing if the controller is broken
    // TODO: log warning
    return;
  } else {
    // At least one port is working
    supported_ports++;
  }

  // Check if we have a second channel (not all machines have a second port)
  if (config & 0x20) {
    // Enable chan 2 briefly
    ps2_write(PS2_COMMAND_PORT, PS2_CMD_ENABLE_CH2);
    // Read config again
    ps2_write(PS2_COMMAND_PORT, PS2_CMD_RECV_CONFIG);
    if (!(ps2_read(PS2_DATA) & 0x20)) {
      // If the "disabled" bit is cleared, chan 2 actually exists
      if (config & 0x20) {
        supported_ports++;
      }
    }
    // Disable it again
    ps2_write(PS2_COMMAND_PORT, PS2_CMD_DISABLE_CH2);
  }

  // Run a test on channel 1 to ensure the keyboard hardware works and is connected
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_TEST_CH1);
  if ((errno = ps2_read(PS2_DATA_PORT)) != 0) {
    // TODO: log warning
  }

  // If a second port is present, too, test it.
  // TODO: chan vs port nomenclature
  if (supported_ports > 1) {
    ps2_write(PS2_COMMAND_PORT, PS2_CMD_TEST_CH2);
    if ((errno = ps2_read(PS2_DATA_PORT)) != 0) {
      // TODO: log warning
    }
  }

  // Next, let's check if we're dealing with a type 1 or type 2 controller

  // Persist the config state
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_RECV_CONFIG);
  config = ps2_read(PS2_DATA_PORT);
  // Next, set the translation bit and update.
  // Translation converts newer scan codes (from the keyboard) to an older format.
  // Setting it helps detect controller type.
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config | 0x40);

  // Now, we check if the translation bit actually stuck.
  // Type 1 controllers support translation; type 2 don't. This helps detect the type.
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_RECV_CONFIG);
  config2 = ps2_read(PS2_DATA_PORT);
  // Restore the original state
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config);
  // Set the type based on whether translation was accepted.
  type = (config2 & 0x40) ? 1 : 0;

  // If chans are present, enable interrupts
  if (supported_ports) {
    // Enable IRQs - 0x01 for chan 1...
    config = 0x01;
    if (supported_ports > 1) {
      // ...0x02 for chan 2
      config |= 0x02;
    }
    // Allow translation
    config |= 0x40;
  } else {
    // Disable clocks if no devices found
    config = 0x10 + 0x20;
  }

  // Apply the final configuration
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config);

  // Clear any leftover data
  ps2_clear_buffer();
  // clang-format off
  // ----------------
  // Query the bus interface type
  // The bus interface type tells us what kind of system architecture or bus the PS/2 controller is attached to.
  // We're checking bit 0 of the result to determine if the system is using:
  // | `iface` value | Bus Type   | Meaning                                                      |
  // | ------------- | ---------- | -------------------------------------------------------------|
  // | 0             | ISA (AT)   | Standard PC/AT (IBM) architecture — what most systems use    |
  // | 1             | MCA (PS/2) | IBM's Micro Channel Architecture — used in IBM PS/2 machines |
  // ----------------
  // clang-format on
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_GET_IFACE);
  unsigned char iface = ps2_read(PS2_DATA_PORT) & 0x01;

  // TODO: Log info

  // Enable the keyboard if possible
  if (supported_ports) {
    ps2_write(PS2_COMMAND_PORT, PS2_CMD_ENABLE_CH1);
    keyboard_init();
  }
}
