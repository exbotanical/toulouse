#ifndef FS_INODE_H
#define FS_INODE_H

typedef struct {
  unsigned short devnum; /* Associated device number */
} inode_t;

typedef struct {
  int __placeholder;
} superblock_t;

#endif /* FS_INODE_H */
