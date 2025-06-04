#ifndef FS_OPS_H
#define FS_OPS_H

#include "fs/dirent.h"
#include "fs/fd.h"
#include "fs/inode.h"
#include "fs/stat.h"
#include "lib/types.h"
#include "proc/proc.h"

typedef struct {
  int flags;
  int fsdev; /* internal filesystem (nodev) */

  /* file operations */
  int (*open)(inode_t *, fd_t *);
  int (*close)(inode_t *, fd_t *);
  int (*read)(inode_t *, fd_t *, char *, size_t);
  int (*write)(inode_t *, fd_t *, const char *, size_t);
  int (*ioctl)(inode_t *, int, unsigned int);
  long long int (*llseek)(inode_t *, long long int);
  int (*readdir)(inode_t *, fd_t *, dirent_t *, size_t);
  int (*readdir64)(inode_t *, fd_t *, dirent64_t *, size_t);
  int (*mmap)(inode_t *, vma_t *);
  int (*select)(inode_t *, int);

  /* inode operations */
  int (*readlink)(inode_t *, char *, size_t);
  int (*followlink)(inode_t *, inode_t *, inode_t **);
  int (*bmap)(inode_t *, unsigned int, int);
  int (*lookup)(const char *, inode_t *, inode_t **);
  int (*rmdir)(inode_t *, inode_t *);
  int (*link)(inode_t *, inode_t *, char *);
  int (*unlink)(inode_t *, inode_t *, char *);
  int (*symlink)(inode_t *, char *, char *);
  int (*mkdir)(inode_t *, char *, unsigned short);
  int (*mknod)(inode_t *, char *, unsigned short, unsigned short);
  int (*truncate)(inode_t *, unsigned int);
  int (*create)(inode_t *, char *, int, unsigned short, inode_t **);
  int (*rename)(inode_t *, inode_t *, inode_t *, inode_t *, char *, char *);

  /* block device I/O operations */
  int (*read_block)(unsigned short, int, char *, int);
  int (*write_block)(unsigned short, int, char *, int);

  /* superblock operations */
  int (*read_inode)(inode_t *);
  int (*write_inode)(inode_t *);
  int (*ialloc)(inode_t *, int);
  void (*ifree)(inode_t *);
  void (*statfs)(superblock_t *, statfs_t *);
  int (*read_superblock)(unsigned short, superblock_t *);
  int (*remount_fs)(superblock_t *, int);
  int (*write_superblock)(superblock_t *);
  void (*release_superblock)(superblock_t *);
} fs_operations_t;

#endif /* FS_OPS_H */
