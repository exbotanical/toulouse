#ifndef CONFIG_H
#define CONFIG_H

/**
 * Minimum percentage of free memory pages
 */
#define FREE_PAGES_RATIO  5

/**
 * Percentage of hash buckets relative to the number of physical pages
 */
#define PAGE_HASH_PER_10K 10

/**
 * Maximum number of pages in hash table
 */
#define MAX_PAGES_HASH    16

#endif /* CONFIG_H */
