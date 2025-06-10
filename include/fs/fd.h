#ifndef FS_FILE_DESCRIPTOR_H
#define FS_FILE_DESCRIPTOR_H

#include "fs/inode.h"

typedef enum {
  FS_TTY_SELECT_FLAG_R = 1,
  FS_TTY_SELECT_FLAG_W = 2,
  FS_TTY_SELECT_FLAG_E = 4,
} fs_tty_select_flag;

typedef struct {
  inode_t*       inode;    /* File inode */
  unsigned short flags;    /* File descriptor flags */
  unsigned short num_open; /* Number of open instances */
} fd_t;

#endif                     /* FS_FILE_DESCRIPTOR_H */
