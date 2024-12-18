#ifndef KSTAT_H
#define KSTAT_H

typedef struct {
  int memsize;
  int extmemsize;
} k_param;

typedef struct {
  k_param      param;
  unsigned int physical_pages;
  unsigned int physical_reserved;
  unsigned int kernel_reserved;
  unsigned int free_pages;
  unsigned int min_free_pages;
  unsigned int total_mem_pages;
} k_stat;

#endif /* KSTAT_H */
