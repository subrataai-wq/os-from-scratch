/*
 * kernel.c - Kernel entry point and early system initialization.
 *
 * Execution environment at kernel_main:
 *  - CPU: x86_64 Long Mode (64-bit).
 *  - Paging: Identity-mapped physical memory provided by firmware page tables.
 *  - Interrupts: Disabled (IF=0). GDT/IDT not yet loaded.
 *  - Display: Linear framebuffer accessible at boot_info->framebuffer.base_address.
 */

#include "kernel.h"

void kernel_main(BootInfo *boot_info)
{
  screen_init(boot_info);
  screen_clear(0x0F172A);

  /* Render title banner */
  screen_fill_rect(0, 0, boot_info->framebuffer.width, 32, 0x1E293B);
  kset_cursor(16, 8);
  kset_colors(COLOR_CYAN, 0x1E293B);
  kprint("  [ MyOS Kernel v0.1.0 ] - 64-bit Bare-Metal Mode");

  kset_colors(COLOR_WHITE, 0x0F172A);

  kset_cursor(16, 48);
  kprint_color("========================================================================\n", COLOR_DARK_GRAY);
  kprint_color("             Welcome to MyOS Kernel - Bare Metal Hand-Off               \n", COLOR_YELLOW);
  kprint_color("========================================================================\n\n", COLOR_DARK_GRAY);

  kprint_color("[  OK  ] ", COLOR_GREEN);
  kprint("UEFI ExitBootServices completed successfully.\n");

  kprint_color("[  OK  ] ", COLOR_GREEN);
  kprint("CPU operating in 64-bit Long Mode (bare-metal environment).\n");

  kprint_color("[  OK  ] ", COLOR_GREEN);
  kprint("Direct framebuffer terminal initialized with 8x16 VGA font.\n\n");

  /* Hardware environment dump */
  kprint_color("--- Hardware Configuration (BootInfo) ---\n", COLOR_YELLOW);

  kprint("  * Framebuffer Address : ");
  kprint_hex(boot_info->framebuffer.base_address);
  kprint(" (");
  kprint_dec(boot_info->framebuffer.width);
  kprint("x");
  kprint_dec(boot_info->framebuffer.height);
  kprint(", Pitch: ");
  kprint_dec(boot_info->framebuffer.pixels_per_scanline);
  kprint(")\n");

  kprint("  * ACPI 2.0+ RSDP      : ");
  kprint_hex(boot_info->rsdp);
  kprint("\n");

  kprint("  * Total System RAM    : ");
  kprint_dec(boot_info->memory_map.total_memory / (1024 * 1024));
  kprint(" MiB (Free: ");
  kprint_dec(boot_info->memory_map.free_memory / (1024 * 1024));
  kprint(" MiB)\n");

  kprint("  * Memory Descriptors  : ");
  kprint_dec(boot_info->memory_map.entry_count);
  kprint(" entries (Firmware MapKey: ");
  kprint_hex(boot_info->memory_map.map_key);
  kprint(")\n");

  kprint("  * Kernel Base Address : ");
  kprint_hex(boot_info->kernel_physical_base);
  kprint(" (Size: ");
  kprint_dec(boot_info->kernel_size);
  kprint(" bytes)\n\n");

  /* Color palette test swatches */
  screen_fill_rect(16, 296, 160, 24, COLOR_RED);
  screen_fill_rect(186, 296, 160, 24, COLOR_GREEN);
  screen_fill_rect(356, 296, 160, 24, COLOR_BLUE);
  screen_fill_rect(526, 296, 160, 24, COLOR_YELLOW);

  kset_cursor(16, 336);
  kprint_color("[  IDLE ] Kernel entering halt loop. Hardware interrupts ready for IDT setup.\n", COLOR_LIGHT_GRAY);

  /* Await interrupts once IDT and timer/keyboard ISRs are installed */
  while (1)
  {
    __asm__ volatile("hlt");
  }
}
