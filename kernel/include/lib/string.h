#ifndef KLIB_STRING_H
#define KLIB_STRING_H

#include "common/types.h"

/**
 * Calculates the length of the string pointed to by s, excluding the
 * terminating null byte.
 *
 * @param s
 * @return size_t the number of bytes in the string pointed to by s
 */
size_t k_strlen(const char* s);

/**
 * Returns a pointer to the first occurrence of the character c in the string s.
 *
 * @param s
 * @param c
 * @return char* a pointer to the matched character or NULL if the character is not found.
 * The terminating null byte is considered part of the string,
 * so that if c is specified as '\0', k_strchr will return a pointer to the terminator.
 */
char* k_strchr(const char* s, char c);

/**
 * Compares the two strings s1 and s2. The locale is not taken into account.
 *
 * @param s1
 * @param s2
 * @return int an integer indicating the result of the comparison, as follows:
 * • 0, if the s1 and s2 are equal;
 * • a negative value if s1 is less than s2;
 * • a positive value if s1 is greater than s2.
 */
int k_strcmp(const char* s1, const char* s2);

/**
 * Copies the string pointed to by src into a string at the buffer pointed to by dest.
 *
 * @param dest
 * @param src
 * @return char* a pointer to dest
 */
char* k_strcpy(char* dest, const char* src);

/**
 * Copies n bytes from memory area src to memory area dest.  The memory areas must not over‐lap.
 *
 * @param dest
 * @param source
 * @param bytes
 * @return void* a pointer to dest
 */
void* k_memcpy(void* dest, const void* source, size_t bytes);

/**
 * Fills the first n bytes of the memory area pointed to by s with the constant byte c.
 *
 * @param s
 * @param c
 * @param bytes
 * @return void* a pointer to the memory area s
 */
void* k_memset(void* s, int c, size_t bytes);

/**
 * Copies n bytes from memory area src to memory area dest. The memory areas may overlap:
 * copying takes place as though the bytes in src are first copied into a temporary array that
 * does  not  overlap src or dest, and the bytes are then copied from the temporary array to dest.
 *
 * @param dest
 * @param src
 * @param bytes
 * @return void* a pointer to dest
 */
void* k_memmove(void* dest, const void* src, size_t bytes);

/**
 * Compares the first n bytes (each interpreted as unsigned char) of the memory areas s1 and s2.
 *
 * @param a
 * @param b
 * @param bytes
 * @return int an integer less than, equal to, or greater than zero if the first n bytes of s1 is
 * found, respectively, to be less than, to match, or be greater than the first n bytes of s2.
 */
int k_memcmp(const void* s1, const void* s2, size_t bytes);

/**
 * Scans the initial n bytes of the memory area pointed to by s for the first instance of c.
 * Both c and the bytes of the memory area pointed to by s are interpreted as unsigned char.
 *
 * @param ptr
 * @param value
 * @param bytes
 * @return void* a pointer to the matching byte or NULL if the character does not occur in the given
 * memory area.
 */
void* k_memchr(void* s, int value, size_t bytes);

/**
 * Converts a string to an integer.
 *
 * @param str
 * @return int
 */
int k_atoi(char* str);

/**
 * Converts an integer to a string.
 * NOTE: This function can only handle the max signed integer size for std c.
 *
 * @param value
 * @param buf
 * @param radix
 * @return char*
 */
char* k_itoa(int value, char* buf, int radix);

#endif /* KLIB_STRING_H */
