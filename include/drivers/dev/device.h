#ifndef DRIVER_DEV_CHAR_DEVICE_H
#define DRIVER_DEV_CHAR_DEVICE_H

#include "fs/ops.h"
#include "lib/types.h"

#define NUM_CHAR_DEVICES                32
#define NUM_BLOCK_DEVICES               32

#define DEVICE_MAJOR(dev)               (((deviceno_t)(dev)) >> 8)
#define DEVICE_MINOR(dev)               (((deviceno_t)(dev)) & 0xFF)
#define DEVICE_MKDEV(major, minor)      (((major) << 8) | (minor))

#define DEVICE_SET_MINOR(minors, bit)   ((minors[(bit) / 32]) |= (1 << ((bit) % 32)))
#define DEVICE_CLEAR_MINOR(minors, bit) ((minors[(bit) / 32]) &= ~(1 << ((bit) % 32)))
#define DEVICE_TEST_MINOR(minors, bit)  ((minors[(bit) / 32]) & (1 << ((bit) % 32)))

typedef enum {
  DEVTYPE_CHAR,
  DEVTYPE_BLOCK,
} devtype_t;

typedef struct device device_t;

struct device {
  char         *name;
  unsigned char major;
  /**
   * bitmap of 256 bits
   */
  unsigned int  minors[8];
  /**
   * default blocksizes for minors, in kb
   * i.e. one entry per minor of that device. where each
   * sub-device advertises its natural I/O block size (e.g. sector size)
   */
  unsigned int *minor_block_sizes;
  /**
   * mostly used for minor sizes, in kb
   */
  void         *device_data;
  // fs_operations_t *fsop;
  void         *requests_queue;
  void         *xfer_data;
  device_t     *next;

  /**
   * File system operations for this device
   */
  fs_operations_t *fs_ops;
};

extern device_t *char_devices_table[NUM_CHAR_DEVICES];
extern device_t *block_devices_table[NUM_BLOCK_DEVICES];

/**
 * TODO:
 */
retval_t device_register(devtype_t type, device_t *new_dev);

/**
 * Initializes the global tables for storing block and char devices.
 */
void devices_init(void);

#endif /* DRIVER_DEV_CHAR_DEVICE_H */
