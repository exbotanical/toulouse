#ifndef FS_FCNTL_H

#define FS_FCNTL_H

/* for open() only */
#define O_CREAT     0100    /* Create file if it does not exist */
#define O_EXCL      0200    /* Exclusive use flag */
#define O_NOCTTY    0400    /* Do not assign controlling terminal */
#define O_TRUNC     01000   /* Truncate flag */
#define O_DIRECTORY 0200000 /* Only open if directory */
#define O_NOFOLLOW  0400000 /* Do not follow symbolic links */

#define O_APPEND    02000
#define O_NONBLOCK  04000
#define O_NDELAY    O_NONBLOCK
#define O_SYNC      010000

#endif /* FS_FCNTL_H */
