#ifndef FS_FILE_DESCRIPTOR_H
#define FS_FILE_DESCRIPTOR_H

#include "fs/inode.h"

typedef struct {
  inode_t*       inode;    /* File inode */
  unsigned short flags;    /* File descriptor flags */
  unsigned short num_open; /* Number of open instances */
} fd_t;

#endif                     /* FS_FILE_DESCRIPTOR_H */
