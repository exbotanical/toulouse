#include "init/multiboot.h"

#include "drivers/console/tmpcon.h"
#include "drivers/video/video.h"
#include "fs/elf.h"
#include "init/bios.h"
#include "kernel.h"
#include "kstat.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/paging.h"

elf32_shdr *symtab;
elf32_shdr *strtab;

static void
multiboot_set_video_props_from_info (multiboot_info_t *mbi) {
  vbe_controller_t *vbe_ctrl = (vbe_controller_t *)mbi->vbe_control_info;
  vbe_mode_t       *vbe_mode = (vbe_mode_t *)mbi->vbe_mode_info;

  video.flags                = VIDEO_MODE_VESAFB;
  video.address              = (unsigned int *)vbe_mode->phys_base;
  video.port                 = 0;
  video.memsize              = vbe_ctrl->total_memory * vbe_mode->win_size * 1024;
  video.columns     = vbe_mode->x_resolution / (vbe_mode->x_char_size ? vbe_mode->x_char_size : 1);
  video.rows        = vbe_mode->y_resolution / (vbe_mode->y_char_size ? vbe_mode->y_char_size : 1);
  video.version     = vbe_ctrl->version;
  video.width       = vbe_mode->x_resolution;
  video.height      = vbe_mode->y_resolution;
  video.char_width  = vbe_mode->x_char_size;
  video.char_height = vbe_mode->y_char_size;
  video.bpp         = vbe_mode->bits_per_pixel;
  video.pixelwidth  = vbe_mode->bits_per_pixel / 8;
  video.pitch       = vbe_mode->bytes_per_scanline;
  video.rowsize     = video.pitch * video.char_height;
  video.size        = vbe_mode->x_resolution * vbe_mode->y_resolution * video.pixelwidth;
  video.vsize       = video.rows * video.pitch * video.char_height;
  kstrcpy((char *)video.signature, (char *)vbe_ctrl->signature);
}

/**
 * Sets default video properties, typically in cases where multiboot was not used by the bootloader.
 * For default, we just use 80x25 VGA.
 */
static void
multiboot_set_video_props_default (void) {
  video.columns = VIDEO_CONS_DEFAULT_COLS;
  video.rows    = VIDEO_CONS_DEFAULT_ROWS;
  video.flags   = VIDEO_MODE_VGA_TXT;
  video.memsize = 384 * 1024;
}

void
multiboot_init (unsigned int magic, unsigned int mbi_ptr) {
  multiboot_info_t mbi;
  kmemset(&video, 0, sizeof(video_props_t));

  // If the bootloader isn't using multiboot, or for some reason the multiboot info was invalid...
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    klogf_warn("invalid multiboot magic number: 0x%x. Assuming 4MB of RAM.\n", magic);

    kmemset(&mbi, 0, sizeof(multiboot_info_t));
    // We default to 640 KB, the legacy x86 convention for max memory:
    // 0 - 640 KB for usable RAM
    // 640 - 1 MB for memory-mapped hardware, BIOS ROMs, video memory, etc
    // See: https://en.wikipedia.org/wiki/Conventional_memory
    kstat.param.memsize    = 640;
    // ~3 MB. Total memory becomes 640 KB (base) + 3072 KB (extended) = ~3.7 MB total
    // This way, the system has at least enough RAM to boot and run basic services, even if exact
    // detection failed.

    // Reminder: Extended memory is typically 1 MB onward aka where the kernel is mapped. That is, 1
    // MB is virtualized as address zero.
    kstat.param.extmemsize = 3072;
    // Initialize default memory map
    bios_mmap_init(NULL, 0);

    multiboot_set_video_props_default();
    return;
  }

  kmemcpy(&mbi, (void *)mbi_ptr, sizeof(multiboot_info_t));

  if (mbi.flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
    klogf_info("bootloader: %s\n", mbi.boot_loader_name);
  }

  if (!(mbi.flags & MULTIBOOT_INFO_MEMORY)) {
    klog_warn("invalid mem_lower, mem_upper values");
  }
  // Set the available RAM size
  kstat.param.memsize    = (unsigned int)mbi.mem_lower;
  kstat.param.extmemsize = (unsigned int)mbi.mem_upper;

  if (!(mbi.flags & MULTIBOOT_INFO_ELF_SHDR)) {
    klog_warn("invalid ELF section header table");
  }

  // Setup memory map info
  if (mbi.flags & MULTIBOOT_INFO_MEM_MAP) {
    bios_mmap_init((multiboot_mmap_entry_t *)mbi.mmap, mbi.mmap_length);
  } else {
    klog_warn("using multiboot but no mmap; we'll use the default");
    bios_mmap_init(NULL, 0);
  }

  // Setup video device info
  if (mbi.flags & MULTIBOOT_INFO_VIDEO_INFO) {
    multiboot_set_video_props_from_info(&mbi);
  }

  // Fallback to standard VGA
  if (!video.flags) {
    multiboot_set_video_props_default();
  }
}

/**
 * Get the last address used by kernel symbols OR the last address of the last module loaded via
 * multiboot. We retrieve this address so we can place the kernel stack beyond these addresses.
 *
 * @param magic The multiboot magick number
 * @param mbr_init The multiboot info pointer
 * @return The last address, as aforementioned.
 */
unsigned int
get_last_boot_addr (unsigned int magic, unsigned int mbr_init) {
  multiboot_info_t *mbi;

  multiboot_elf_section_header_table_t *hdr;
  // Round the address down to the nearest lower page boundary, then setup the next page to avoid
  // the kernel's static data.
  unsigned int                          addr = ((unsigned int)image_end & PAGE_MASK) + PAGE_SIZE;

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return P2V(addr);
  }

  mbi = (multiboot_info_t *)mbr_init;

  // ELF header tables
  if (mbi->flags & MULTIBOOT_INFO_ELF_SHDR) {
    symtab = NULL;
    strtab = NULL;
    hdr    = &(mbi->u.elf_sec);
    for (unsigned short int n = 0; n < hdr->num; n++) {
      elf32_shdr *shdr = (elf32_shdr *)(hdr->addr + (n * hdr->size));
      if (shdr->sh_type == SHT_SYMTAB) {
        symtab = shdr;
      }
      if (shdr->sh_type == SHT_STRTAB) {
        strtab = shdr;
      }
    }

    addr = max(addr, strtab->sh_addr + strtab->sh_size);
  }

  if (mbi->flags & MULTIBOOT_INFO_MODS) {
    multiboot_module_t *mod = (multiboot_module_t *)mbi->mods;
    for (unsigned short int n = 0; n < mbi->mods_count; n++, mod++) {
      addr = max(addr, mod->end);
    }
  }

  return P2V(addr);
}
