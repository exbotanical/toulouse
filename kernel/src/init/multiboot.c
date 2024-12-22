#include "init/multiboot.h"

#include "drivers/console/vga.h"
#include "drivers/dev/char/console.h"
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
  k_strcpy((char *)video.signature, (char *)vbe_ctrl->signature);
}

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
  k_memset(&video, 0, sizeof(video_props_t));

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    vgaprintf("[WARN]: invalid multiboot magic number: 0x%x. Assuming 4MB of RAM.\n", magic);

    k_memset(&mbi, 0, sizeof(multiboot_info_t));
    kstat.param.memsize    = 640;
    kstat.param.extmemsize = 3072;
    bios_mmap_init(NULL, 0);
    multiboot_set_video_props_default();
    return;
  }

  k_memcpy(&mbi, (void *)mbi_ptr, sizeof(multiboot_info_t));

  if (mbi.flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
    vgaprintf("bootloader: %s\n", mbi.boot_loader_name);
  }

  if (!(mbi.flags & MULTIBOOT_INFO_MEMORY)) {
    vgaprintf("%s\n", "[WARN]: invalid mem_lower, mem_upper values");
  }
  kstat.param.memsize    = (unsigned int)mbi.mem_lower;
  kstat.param.extmemsize = (unsigned int)mbi.mem_upper;

  if (!(mbi.flags & MULTIBOOT_INFO_ELF_SHDR)) {
    vgaprintf("%s\n", "[WARN]: invalid ELF section header table");
  }

  if (mbi.flags & MULTIBOOT_INFO_MEM_MAP) {
    bios_mmap_init((multiboot_mmap_entry_t *)mbi.mmap, mbi.mmap_length);
  } else {
    bios_mmap_init(NULL, 0);
  }

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
