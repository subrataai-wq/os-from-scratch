/*
 * screen.h - Bare-metal linear framebuffer console and graphics primitives.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <stddef.h>
#include "../boot_info.h"

/* Canonical 24-bit RGB (0x00RRGGBB) color definitions */
#define COLOR_BLACK       0x000000
#define COLOR_DARK_GRAY   0x303030
#define COLOR_LIGHT_GRAY  0xA0A0A0
#define COLOR_WHITE       0xFFFFFF
#define COLOR_RED         0xFF4444
#define COLOR_GREEN       0x44FF44
#define COLOR_BLUE        0x4488FF
#define COLOR_YELLOW      0xFFFF44
#define COLOR_CYAN        0x33FFFF
#define COLOR_MAGENTA     0xFF33FF
#define COLOR_NAVY        0x001122

/* Display initialization and primitive rendering */
void screen_init(BootInfo *boot_info);
void screen_clear(uint32_t color);
void screen_putpixel(uint32_t x, uint32_t y, uint32_t rgb24);
void screen_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb24);
void screen_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);

/* Kernel terminal formatting */
void kprint(const char *str);
void kprint_color(const char *str, uint32_t fg);
void kprint_hex(uint64_t val);
void kprint_dec(uint64_t val);
void kset_cursor(uint32_t x, uint32_t y);
void kset_colors(uint32_t fg, uint32_t bg);

#endif /* SCREEN_H */
