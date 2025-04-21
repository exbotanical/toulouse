#ifndef KSTAT_H
#define KSTAT_H

/**
 * Represents configurable kernel parameters.
 */
typedef struct {
  /**
   * Base memory in KB, conventionally the first 640 KB of RAM. That is, this is the amount of lower
   * memory in KB. Size of usable RAM below 1 MB
   */
  int memsize;
  /**
   * Extended memory in KB, above 1 MB and up to the system limit. That is, this is the amount of
   * memory above 1 MB. Size of usable RAM above 1 MB
   */
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

  unsigned int ticks;       /* ticks (1/HZths of sec) since boot */
  unsigned int system_time; /* current system time (since the Epoch) */
  unsigned int uptime;      /* seconds since boot */
} kstat_t;

#endif                      /* KSTAT_H */
