#include "drivers/dev/device.h"

#include "lib/string.h"

device_t *char_devices_table[NUM_CHAR_DEVICES];
device_t *block_devices_table[NUM_BLOCK_DEVICES];

retval_t
device_register (devtype_t type, device_t *new_dev) {
  // TODO:
  return RET_OK;
}

void
devices_init (void) {
  kmemset(char_devices_table, 0, sizeof(char_devices_table));
  kmemset(block_devices_table, 0, sizeof(block_devices_table));
}
