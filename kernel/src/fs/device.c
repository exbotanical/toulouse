#include "fs/device.h"

#include "lib/string.h"

device_t *char_devices_table[NUM_CHAR_DEVICES];
device_t *block_devices_table[NUM_BLOCK_DEVICES];

void
devices_init (void) {
  k_memset(char_devices_table, 0, sizeof(char_devices_table));
  k_memset(block_devices_table, 0, sizeof(block_devices_table));
}
