#ifndef FS_DEVICE_H
#define FS_DEVICE_H

#define NUM_CHAR_DEVICES  32
#define NUM_BLOCK_DEVICES 32

typedef struct device device_t;

struct device {
  char         *name;
  unsigned char major;
  /**
   * bitmap of 256 bits
   */
  unsigned int  minors[8];
  /**
   * default minor blocksizes, in kb
   */
  unsigned int *block_size;
  /**
   * mostly used for minor sizes, in kb
   */
  void         *device_data;
  // fs_operations_t *fsop;
  void         *requests_queue;
  void         *xfer_data;
  device_t     *next;
};

extern device_t *char_devices_table[NUM_CHAR_DEVICES];
extern device_t *block_devices_table[NUM_BLOCK_DEVICES];

void devices_init(void);

#endif /* FS_DEVICE_H */
