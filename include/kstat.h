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
  kparam_t param;
  int      physical_pages;
  int      physical_reserved;
  int      kernel_reserved;
  /**
   * The number of free pages in the free page list
   */
  int      num_free_pages;
  int      min_free_pages;
  int      total_mem_pages;
  // Pages last reclaimed from the buffer
  int      pages_reclaimed;

  /**
   * The amount of memory used for caching file pages
   */
  int page_cache_consumption;

  unsigned int ticks;       /* ticks (1/HZths of sec) since boot */
  unsigned int system_time; /* current system time (since the Epoch) */
  unsigned int uptime;      /* seconds since boot */
} kstat_t;

#endif                      /* KSTAT_H */
