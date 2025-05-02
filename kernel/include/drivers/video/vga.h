#ifndef VIDEO_DEV_VGA_H
#define VIDEO_DEV_VGA_H

#define VGA_MONO_ADDR           0xB0000L
#define VGA_COLOR_ADDR          0xB8000L

/* Motorola MC6845 CRT Controller I/O ports */
#define VGA_MONO_PORT           0x3B4
#define VGA_COLOR_PORT          0x3D4

/* attribute controller register */
#define VGA_ATTR_CONTROLLER     0x3C0
/* palette address source */
#define VGA_ATTR_CONTROLLER_PAS 0x20
/* input status #1 register */
#define VGA_INPUT_STAT1         0x3DA
#define VGA_MONO_STAT_PORT      0x3BA

#define VGA_MONO_SIG            "VGA monochrome 80x25"
#define VGA_COLOR_SIG           "VGA color 80x25"

/**
 * Selects the index port of the CRT controller (used to choose which internal register we're
 * accessing).
 *
 * Reminder: Since there's only one data port but many different internal registers, we first need
 * to tell the VGA controller which internal register we're talking about when doing a write.
 */
#define CRT_INDEX               0
/* Selects the data port of the CRT controller (used to r/w the selected register). */
#define CRT_DATA                1
/* Sets the cursor start scanline (top row of the cursor block within a character cell). */
#define CRT_CURSOR_STR          0xA
/* Sets the cursor end scanline (bottom row of the cursor block). */
#define CRT_CURSOR_END          0xB
/**
 * High byte of the video memory start address — controls where the top-left corner of the visible
 * screen begins in video memory.
 */
#define CRT_START_ADDR_HI       0xC
/* Low byte of the start address. */
#define CRT_START_ADDR_LO       0xD
/* High byte of the cursor position (i.e., where the blinking cursor is shown). */
#define CRT_CURSOR_POS_HI       0xE
/* Low byte of the cursor position. */
#define CRT_CURSOR_POS_LO       0xF

/* Indicates the console is using VGA text mode */
#define VPF_VGA                 0x01
/**
 * Indicates the console is using a VESA framebuffer. This refers to a linear framebuffer mode
 * provided by the VESA BIOS Extensions (VBE), which allows more advanced graphics (e.g., higher
 * resolutions and color depths) than legacy VGA text modes.
 */
#define VPF_VESAFB              0x02
/* Indicates the text-mode cursor is currently visible on screen */
#define VPF_CURSOR_ON           0x04

/**
 * Initializes the video struct fields using the VGA driver.
 * Also sets either mono or color VGA mode
 * depending on which bits are set in the BIOS Data Area.
 */
void vga_init(void);

#endif /* VIDEO_DEV_VGA_H */
