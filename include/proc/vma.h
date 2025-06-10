#ifndef PROC_VMA_H
#define PROC_VMA_H

#include "fs/inode.h"
#include "lib/types.h"
#include "mem/segments.h"

/**
 * Represents a Virtual Memory Area — a contiguous region of virtual memory in a process's address
 * space, associated with some backing (e.g. a file, shared memory, or anonymous mapping).
 *
 * This structure allows the kernel to track each distinct mapped region in a process' virtual
 * address space — what file or object it's associated with, what protections and semantics it has,
 * and how to find others.
 */
typedef struct vma vm_area_t;

struct vma {
  /**
   * Start address (inclusive) of the virtual memory region
   */

  unsigned int start;
  /**
   * End address (exclusive) of the region
   */
  unsigned int end;

  /**
   * Protection flags, indicating allowed access e.g. PROT_READ, PROT_WRITE, ...
   * Controls what kind of memory access is allowed
   */
  char prot;

  /**
   * Flags indicating how the mapping behaves with respect to other processes or the file system
   * e.g. MAP_SHARED, MAP_PRIVATE, ...
   */
  unsigned int flags;

  /**
   * Offset into the backing file (if inode is non-null)
   */
  unsigned int offset;

  /**
   * Segment type (P_TEXT, P_DATA, ...)
   * Helps categorize the VMA semantically
   */
  segment_type seg_type;

  /**
   * Backing file's inode, if the mapping is file-based
   */
  inode_t *inode;

  /**
   * File open mode for this mapping (O_RDONLY, O_RDWR, ...).
   * Determines how the process opened the file and what access it has
   */
  char o_mode;

  /**
   * Feneric pointer for extended use.
   * Currently used for shared memory objects (shm).
   */
  void *object;

  /* Doubly linked list pointers to adjacent VMAs in the process */
  vm_area_t *prev;
  vm_area_t *next;
};

typedef enum {
  VERIFY_READ  = 1,
  VERIFY_WRITE = 2,
} verify_addr_type;

int        verify_address(verify_addr_type type, const void *addr, unsigned int size);
vm_area_t *find_vma_region(unsigned int addr);

#endif /* PROC_VMA_H */
