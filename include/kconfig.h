#ifndef CONFIG_H
#define CONFIG_H

/**
 * Minimum percentage of free memory pages
 */
#define FREE_PAGES_RATIO   5

/**
 * Percentage of hash buckets relative to the number of physical pages
 */
#define PAGE_HASH_PER_10K  10

/**
 * Maximum number of pages in hash table
 */
#define MAX_PAGES_HASH     16

/**
 * Maximum number of concurrent processes.
 */
#define NUM_PROCS          64

/**
 * Maximum number of active timer tasks.
 */
#define NUM_TIMER_TASKS    NUM_PROCS

/**
 * The number of buffers reclaimed at once
 */
#define NUM_BUFFER_RECLAIM 250


#define NUM_CONSOLES      12
#define NUM_SYSCONSOLES   1

#endif /* CONFIG_H */
