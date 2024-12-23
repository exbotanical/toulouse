#ifndef KSTAT_H
#define KSTAT_H

/**
 * Represents configurable kernel parameters.
 */
typedef struct {
  int memsize;
  int extmemsize;
} kparam_t;

/**
 * Kernel-wide stats and state.
 */
typedef struct {
  kparam_t     param;
  unsigned int physical_pages;
  unsigned int physical_reserved;
  unsigned int kernel_reserved;
  unsigned int free_pages;
  unsigned int min_free_pages;
  unsigned int total_mem_pages;
} kstat_t;

#endif /* KSTAT_H */
