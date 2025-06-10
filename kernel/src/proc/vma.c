#include "proc/vma.h"

#include "lib/errno.h"
#include "mem/mman.h"
#include "mem/page.h"

int
verify_address (verify_addr_type type, const void *addr, unsigned int size) {
  if (!current->vma_table) {
    return 0;
  }

  unsigned int start = (unsigned int)addr;
  vm_area_t   *vma;
  if (!(vma = find_vma_region(start))) {
    // Check whether addr looks like a non-existent user stack address.
    // If so, return false and let `trap_page_fault` handle the imminent page fault
    vma = proc_current->vma_table->prev;
    if (vma) {
      if (vma->seg_type == SEGTYPE_STACK) {
        if (start < vma->start && start > vma->prev->end) {
          return 0;
        }
      }
    }
    return -EFAULT;
  }

  for (;;) {
    if (type == VERIFY_WRITE) {
      if (!(vma->prot & PROT_WRITE)) {
        return -EFAULT;
      }
    } else {
      if (!(vma->prot & PROT_READ)) {
        return -EFAULT;
      }
    }
    if (start + size < vma->end) {
      break;
    }
    if (!(vma = find_vma_region(vma->end))) {
      return -EFAULT;
    }
  }

  return 0;
}

vm_area_t *
find_vma_region (unsigned int addr) {
  if (!addr) {
    return NULL;
  }

  addr           &= PAGE_MASK;
  vm_area_t *vma  = proc_current->vma_table;

  while (vma) {
    if ((addr >= vma->start) && (addr < vma->end)) {
      return vma;
    }
    vma = vma->next;
  }
  return NULL;
}
