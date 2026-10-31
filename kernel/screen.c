/*
 * screen.c - Direct linear framebuffer renderer and text terminal.
 *
 * Implements hardware-independent pixel drawing, clipped rectangle fills,
 * 8x16 VGA font glyph blitting, and automated terminal scrolling across
 * arbitrary GOP resolutions and scanline strides.
 */

#include "screen.h"
#include "font.h"

static FramebufferInfo g_fb;
static uint32_t g_cursor_x = 16;
static uint32_t g_cursor_y = 16;
static uint32_t g_fg_color = COLOR_WHITE;
static uint32_t g_bg_color = COLOR_NAVY;

/*
 * Translate canonical 0x00RRGGBB into native 32-bit hardware pixel values.
 * UEFI GOP displays typically configure BGRX8888, with occasional RGBX8888.
 */
static uint32_t rgb_to_native(uint32_t rgb24)
{
  uint32_t r = (rgb24 >> 16) & 0xFF;
  uint32_t g = (rgb24 >> 8) & 0xFF;
  uint32_t b = rgb24 & 0xFF;

  if (g_fb.red_mask == 0x00FF0000 && g_fb.blue_mask == 0x000000FF)
  {
    return (r << 16) | (g << 8) | b;
  }
  else if (g_fb.red_mask == 0x000000FF && g_fb.blue_mask == 0x00FF0000)
  {
    return (b << 16) | (g << 8) | r;
  }

  return (r << 16) | (g << 8) | b;
}

void screen_init(BootInfo *boot_info)
{
  if (!boot_info)
    return;
  g_fb = boot_info->framebuffer;
  g_cursor_x = 16;
  g_cursor_y = 16;
  g_fg_color = COLOR_WHITE;
  g_bg_color = COLOR_NAVY;
}

void screen_putpixel(uint32_t x, uint32_t y, uint32_t rgb24)
{
  if (x >= g_fb.width || y >= g_fb.height || g_fb.base_address == 0)
    return;

  volatile uint32_t *fb = (volatile uint32_t *)g_fb.base_address;
  fb[y * g_fb.pixels_per_scanline + x] = rgb_to_native(rgb24);
}

void screen_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t rgb24)
{
  if (g_fb.base_address == 0 || g_fb.pixels_per_scanline == 0)
    return;

  uint32_t native_val = rgb_to_native(rgb24);
  volatile uint32_t *fb = (volatile uint32_t *)g_fb.base_address;

  uint32_t max_x = (x + w > g_fb.width) ? g_fb.width : (x + w);
  uint32_t max_y = (y + h > g_fb.height) ? g_fb.height : (y + h);

  for (uint32_t cy = y; cy < max_y; cy++)
  {
    uint32_t row_offset = cy * g_fb.pixels_per_scanline;
    for (uint32_t cx = x; cx < max_x; cx++)
    {
      fb[row_offset + cx] = native_val;
    }
  }
}

void screen_clear(uint32_t rgb24)
{
  g_bg_color = rgb24;
  screen_fill_rect(0, 0, g_fb.width, g_fb.height, rgb24);
  g_cursor_x = 16;
  g_cursor_y = 16;
}

/*
 * Expand 1-bpp font glyph bitmap onto the 32-bpp linear framebuffer.
 */
void screen_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg)
{
  if (g_fb.base_address == 0)
    return;

  uint8_t uc = (uint8_t)c;
  uint32_t native_fg = rgb_to_native(fg);
  uint32_t native_bg = rgb_to_native(bg);
  volatile uint32_t *fb = (volatile uint32_t *)g_fb.base_address;

  for (uint32_t row = 0; row < FONT_HEIGHT; row++)
  {
    uint8_t row_bits = font8x16[uc][row];
    uint32_t row_offset = (y + row) * g_fb.pixels_per_scanline;

    if (y + row >= g_fb.height)
      break;

    for (uint32_t col = 0; col < FONT_WIDTH; col++)
    {
      if (x + col >= g_fb.width)
        break;

      if ((row_bits >> (7 - col)) & 1)
      {
        fb[row_offset + (x + col)] = native_fg;
      }
      else
      {
        fb[row_offset + (x + col)] = native_bg;
      }
    }
  }
}

/*
 * Shift framebuffer rows upward by one font line height when reaching display boundary.
 */
static void scroll_if_needed(void)
{
  if (g_cursor_y + FONT_HEIGHT + 16 > g_fb.height)
  {
    /* Guard: Prevent uint32_t underflow on compact or non-standard resolutions */
    if (g_fb.height <= (FONT_HEIGHT + 32) || g_fb.base_address == 0 || g_fb.pixels_per_scanline == 0)
    {
      screen_clear(g_bg_color);
      g_cursor_x = 16;
      g_cursor_y = 16;
      return;
    }

    volatile uint32_t *fb = (volatile uint32_t *)g_fb.base_address;
    uint32_t total_rows_to_copy = g_fb.height - (FONT_HEIGHT + 16);

    for (uint32_t cy = 16; cy < total_rows_to_copy; cy++)
    {
      uint32_t dst_offset = cy * g_fb.pixels_per_scanline;
      uint32_t src_offset = (cy + FONT_HEIGHT) * g_fb.pixels_per_scanline;
      for (uint32_t cx = 16; cx < g_fb.width - 16; cx++)
      {
        fb[dst_offset + cx] = fb[src_offset + cx];
      }
    }

    screen_fill_rect(16, total_rows_to_copy, g_fb.width - 32, FONT_HEIGHT, g_bg_color);
    g_cursor_y -= FONT_HEIGHT;
  }
}

void kprint_color(const char *str, uint32_t fg)
{
  if (!str)
    return;

  while (*str)
  {
    char c = *str++;
    if (c == '\n')
    {
      g_cursor_x = 16;
      g_cursor_y += FONT_HEIGHT;
      scroll_if_needed();
    }
    else if (c == '\r')
    {
      g_cursor_x = 16;
    }
    else if (c == '\t')
    {
      g_cursor_x += FONT_WIDTH * 4;
    }
    else
    {
      screen_draw_char(g_cursor_x, g_cursor_y, c, fg, g_bg_color);
      g_cursor_x += FONT_WIDTH;

      if (g_cursor_x + FONT_WIDTH + 16 > g_fb.width)
      {
        g_cursor_x = 16;
        g_cursor_y += FONT_HEIGHT;
        scroll_if_needed();
      }
    }
  }
}

void kprint(const char *str)
{
  kprint_color(str, g_fg_color);
}

void kprint_hex(uint64_t val)
{
  char buf[19];
  buf[0] = '0';
  buf[1] = 'x';
  for (int i = 0; i < 16; i++)
  {
    uint8_t nibble = (uint8_t)((val >> (60 - 4 * i)) & 0x0F);
    buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
  }
  buf[18] = '\0';
  kprint(buf);
}

void kprint_dec(uint64_t val)
{
  char buf[21];
  int i = 20;
  buf[i] = '\0';
  do
  {
    buf[--i] = '0' + (char)(val % 10);
    val /= 10;
  } while (val > 0);
  kprint(&buf[i]);
}

void kset_cursor(uint32_t x, uint32_t y)
{
  g_cursor_x = x;
  g_cursor_y = y;
}

void kset_colors(uint32_t fg, uint32_t bg)
{
  g_fg_color = fg;
  g_bg_color = bg;
}
