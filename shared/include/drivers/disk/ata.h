#ifndef DISK_H
#define DISK_H

#include "common/types.h"

#define ATA_SECTOR_SZ           512

#define ATA_PORT_DATA           0x1F0
#define ATA_PORT_ERR            0x1F1
#define ATA_PORT_SECCOUNT       0x1F2
#define ATA_PORT_LBA_LOW        0x1F3
#define ATA_PORT_LBA_MID        0x1F4
#define ATA_PORT_LBA_HI         0x1F5
#define ATA_PORT_DRIVE_HEAD     0x1F6
#define ATA_PORT_COMM_REGSTAT   0x1F7
#define ATA_PORT_ALTSTAT_DCR    0x3F6

#define ATA_CMD_PIO_READ        0x20
#define ATA_CMD_PIO_READ_EXT    0x24
#define ATA_CMD_PIO_WRITE       0x30
#define ATA_CMD_PIO_WRITE_EXT   0x34
#define ATA_CMD_DMA_READ        0xC8
#define ATA_CMD_DMA_READ_EXT    0x25
#define ATA_CMD_DMA_WRITE       0xCA
#define ATA_CMD_DMA_WRITE_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_SET_FEATURES    0xEF

// Indicates an error occurred. Send a new command to clear it.
#define ATA_STAT_ERR            (1 << 0)
// The drive has PIO data to transfer, or is ready to accept PIO data.
#define ATA_STAT_DRQ            (1 << 3)
// Overlapped Mode Service Request.
#define ATA_STAT_SRV            (1 << 4)
// Drive Fault Error (does not set ERR).
#define ATA_STAT_DF             (1 << 5)
// Bit is clear when drive is spun down, or after an error. Set otherwise.
#define ATA_STAT_RDY            (1 << 6)
// The drive is preparing to send/receive data (wait for it to clear).
#define ATA_STAT_BSY            (1 << 7)

/**
 * Waits until the ATA drive is ready to accept commands via the I/O port.
 *
 * Note: This consumes CPU cycles and should not be used when multiple processes
 * are being scheduled. That is, restrict usage to the bootloader or early on in
 * the kernel's lifecycle.
 */
void ata_wait_disk(void);

/**
 * Reads a sector starting at `offset` from disk and into `dest` using the ATA
 * PIO mode.
 *
 * @param dest destination address for the block of data that is read
 * @param offset the LBA i.e. offset at which the target sector begins
 */
void ata_read_sector(void *dest, uint32_t offset);

/**
 * Reads `c` bytes starting at `offset` from disk and into `dest` using the
 * ATA PIO mode.
 *
 * @param dest physical address where data should be copied
 * @param c number of bytes to read
 * @param offset where on disk the read begins
 */
void ata_read_segment(uint8_t *dest, uint32_t c, uint32_t offset);

#endif /* DISK_H */
