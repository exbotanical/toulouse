#ifndef ARCH_X86_H
#define ARCH_X86_H

#include "lib/types.h"

#define IO_BITMAP_SIZE 8192

/* Intel 386 Task Switch State */
typedef struct {
  unsigned int       prev_tss;
  unsigned int       esp0;
  unsigned int       ss0;
  unsigned int       esp1;
  unsigned int       ss1;
  unsigned int       esp2;
  unsigned int       ss2;
  unsigned int       cr3;
  unsigned int       eip;
  unsigned int       eflags;
  unsigned int       eax;
  unsigned int       ecx;
  unsigned int       edx;
  unsigned int       ebx;
  unsigned int       esp;
  unsigned int       ebp;
  unsigned int       esi;
  unsigned int       edi;
  unsigned int       es;
  unsigned int       cs;
  unsigned int       ss;
  unsigned int       ds;
  unsigned int       fs;
  unsigned int       gs;
  unsigned int       ldt;
  unsigned short int debug_trap;
  unsigned short int io_bitmap_addr;
  unsigned char      io_bitmap[IO_BITMAP_SIZE + 1];
} i386tss_t;

// See: https://wiki.osdev.org/I/O_Ports

/**
 * Reads a byte of data from the given I/O port.
 *
 * @param port 16 bit port number
 * @return uint8_t
 */
static inline uint8_t
inb (uint16_t port) {
  volatile uint8_t retval;
  asm volatile("inb %1, %0" : "=a"(retval) : "Nd"(port));
  return retval;
}

/**
 * Writes a byte of data to the given I/O port.
 *
 * @param port 16 bit port number
 * @param data data to write
 */
static inline void
outb (uint16_t port, uint8_t data) {
  asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * Reads a word of data from the given I/O port.
 *
 * @param port 16 bit port number
 * @return uint16_t
 */
static inline uint16_t
inw (uint16_t port) {
  volatile uint16_t retval;
  asm volatile("inw %1, %0" : "=a"(retval) : "Nd"(port));
  return retval;
}

/**
 * Writes a word of data to the given I/O port.
 *
 * @param port 16 bit port number
 * @param data data to write
 */
static inline void
outw (uint16_t port, uint16_t data) {
  asm volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * Reads 32 bits of data from the given I/O port.
 *
 * @param port 16 bit port number
 * @return uint32_t
 */
static inline uint16_t
inl (uint16_t port) {
  volatile uint32_t retval;
  asm volatile("inl %1, %0" : "=a"(retval) : "Nd"(port));
  return retval;
}

/**
 * Writes 32 bits of data to the given I/O port.
 *
 * @param port 16 bit port number
 * @param data data to write
 */
static inline void
outl (uint16_t port, uint32_t data) {
  asm volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * Reads a block of data from the given I/O port.
 *
 * @param port The I/O port from which data is read
 * @param addr The starting memory address of the data to be read
 * @param c The number of 32-bit words to read
 */
static inline void
insl (uint16_t port, void *addr, uint32_t c) {
  // Repeatedly execute insl until c == 0.
  // Outputs:
  //    =D -> %edi, mem address
  //    =c -> %ecx, counter
  // Inputs:
  //    d -> %edx, I/O port
  //    0 -> reuse the addr output operand
  //    1 -> reuse the c output operand
  // Clobbered:
  //    memory -> we're modifying memory
  //    cc -> cond codes (EFLAGS) will be modified
  asm volatile("cld; rep insl"
               : "=D"(addr), "=c"(c)
               : "d"(port), "0"(addr), "1"(c)
               : "memory", "cc");
}

/**
 * Writes a block of data to the given I/O port.
 *
 * @param port The I/O port to which data is written
 * @param addr The address into which the data will be read
 * @param c The number of 32-bit double-words to write
 */
static inline void
outsl (uint16_t port, const void *addr, uint32_t c) {
  asm volatile("cld; rep outsl" : "=S"(addr), "=c"(c) : "d"(port), "0"(addr), "1"(c) : "cc");
}

/**
 * Stores `c` bytes of data in memory starting at `addr`.
 *
 * @param addr
 * @param data
 * @param c
 */
static inline void
stosb (void *addr, int32_t data, int32_t c) {
  asm volatile("cld; rep stosb"
               : "=D"(addr), "=c"(c)
               : "0"(addr), "1"(c), "a"(data)
               : "memory", "cc");
}

static inline void
halt (void) {
  asm volatile("hlt");
}

/**
 * Forces the compiler to NOT reorder around the barrier in either direction such that no operation
 * before the barrier can reorder with any operation after the barrier (and vice versa).
 */
static inline void
barrier (void) {
  asm volatile("" ::: "memory");
}

/**
 * Idles the CPU via a no-op instruction. Uses `rep nop` aka OpCode F390 aka "pause", optimized on
 * x86.
 */
static inline void
idle (void) {
  asm volatile("rep; nop" ::: "memory");
}

#endif /* ARCH_X86_H */
