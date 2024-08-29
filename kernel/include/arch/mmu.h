#ifndef MMU_H
#define MMU_H

// Control Register flags
#define CR0_PE  0x00000001  // Protection Enable
#define CR0_WP  0x00010000  // Write Protect
#define CR0_PG  0x80000000  // Paging

#define CR4_PSE 0x00000010  // Page size extension

#endif                      /* MMU_H */
