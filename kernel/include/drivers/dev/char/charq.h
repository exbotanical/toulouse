#ifndef DRIVERS_DEV_CHAR_CHARQ_H
#define DRIVERS_DEV_CHAR_CHARQ_H

#include "common/types.h"

#define CBLOCK_SIZE 32
#define CHARQ_SIZE  8

/**
 * Character buffer block.
 */
typedef struct cblock cblock_t;

struct cblock {
  /**
   * Represents the next index to read from.
   * This is effectively the start index of this cblock buffer.
   */
  unsigned short next_read_index;
  /**
   * Represents the next index to write to.
   * This is effectively the end index of this cblock buffer.
   */
  unsigned short next_write_index;
  /**
   * Characters
   */
  unsigned char  data[CBLOCK_SIZE];

  cblock_t *prev;
  cblock_t *next;
};

/**
 * A linked list of character buffers
 */
typedef struct {
  /**
   * The number of characters in the current cblock buffer, where the current cblock is invariably
   * the tail.
   */
  unsigned short current_cblock_size;
  /**
   * The number of cblocks in this list
   */
  unsigned short size;

  cblock_t *head;
  cblock_t *tail;
} charq_t;

/**
 * Appends a character to the end of a character queue.
 * If the current tail `cblock` is full, a new `cblock_t` is allocated and linked to the tail.
 *
 * @return RET_OK on success, or RET_FAIL if memory allocation fails.
 */
retval_t charq_put_char(charq_t *q, unsigned char c);

/**
 * Removes the last character from the end of a character queue and returns it.
 *
 * This function retrieves and removes the last character in the character
 * queue. If the last `cblock` becomes empty after the removal, it is deleted
 * from the queue. If the queue is empty, 0 is returned.
 */
retval_t charq_unput_char(charq_t *q);

/**
 * Returns the last character in the queue.
 */
unsigned char charq_get_char(charq_t *q);

/**
 * Purges the character queue.
 *
 * This function resets the character queue by effectively removing all characters
 * currently stored in the queue.
 */
void charq_flush(charq_t *q);

/**
 * Returns the remaining number of chars the queue can accommodate.
 */
int charq_remaining(charq_t *q);

#endif /* DRIVERS_DEV_CHAR_CHARQ_H */
