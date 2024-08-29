#include "drivers/disk/ata.h"

#include "arch/x86.h"

void
ata_wait_disk (void) {
  // 0xC0: This mask corresponds to the bits in the ATA status register that
  // indicate the status of the drive.
  // 0x80: BSY bit. If this bit is set (1), the drive is busy with a command and
  // cannot accept new commands.
  // 0x40: RDY bit, indicates that the drive is ready. This bit must be
  // set (1) for the drive to accept a new command.
  while ((inb(ATA_PORT_COMM_REGSTAT) & 0xC0) != ATA_STAT_RDY);
}

void
ata_read_sector (void *dest, uint32_t offset) {
  // Make sure the disk is ready
  ata_wait_disk();

  // Send the number of sectors to read
  outb(ATA_PORT_SECCOUNT, 1);
  // Send the low byte of the LBA
  // The sector LBA is split into several parts. For this one, the ATA needs to
  // know where the sector begins.
  outb(ATA_PORT_LBA_LOW, offset);
  outb(ATA_PORT_LBA_MID, offset >> 8);
  outb(ATA_PORT_LBA_HI, offset >> 16);
  // Select master drive, OR'd with the high bits of the LBA
  outb(ATA_PORT_DRIVE_HEAD, (offset >> 24) | 0xE0);
  outb(ATA_PORT_COMM_REGSTAT, ATA_CMD_PIO_READ);

  // Wait until ready again, then read the data
  ata_wait_disk();
  insl(ATA_PORT_DATA, dest, ATA_SECTOR_SZ);
}

void
ata_read_segment (uint8_t *dest, uint32_t c, uint32_t offset) {
  // Note `dest` is a uint8_t so we can do proper pointer arithmetic
  // e.g. `dest + 1` means increment by one byte
  uint8_t  end_phys_addr   = dest + c;
  // Round down to the nearest sector boundary
  uint8_t  start_phys_addr = dest - (offset % ATA_SECTOR_SZ);
  // Derive the sector number; we add 1 to account for the kernel, which resides
  // at sector 1.
  uint32_t sector_num      = (offset / ATA_SECTOR_SZ) + 1;
  // Read each sector sequentially
  for (; start_phys_addr < end_phys_addr;
       start_phys_addr += ATA_SECTOR_SZ, offset++) {
    ata_read_sector(start_phys_addr, offset);
  }
}
