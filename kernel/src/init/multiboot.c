#include "init/multiboot.h"

#include "drivers/console/vga.h"
#include "drivers/dev/char/console.h"
#include "init/bios.h"
#include "kernel.h"
#include "lib/string.h"

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
    k_param_memsize    = 640;
    k_param_extmemsize = 3072;
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
  k_param_memsize    = (unsigned int)mbi.mem_lower;
  k_param_extmemsize = (unsigned int)mbi.mem_upper;

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
