/*
 * boot_info.h - Firmware-to-kernel handoff contract.
 *
 * Defines the unified runtime environment structure populated by the UEFI
 * bootloader before ExitBootServices() and consumed by kernel_main().
 *
 * Guarantees at kernel entry:
 *  - 64-bit Long Mode active with paging enabled.
 *  - Physical memory identity-mapped across all available RAM regions.
 *  - Linear framebuffer accessible via direct physical addressing.
 *  - Interrupts disabled (IF=0).
 */

#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   * Linear framebuffer configuration acquired from Graphics Output Protocol (GOP).
   * pixels_per_scanline accounts for stride pitch padding required by display hardware.
   */
  typedef struct
  {
    uint64_t base_address;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
    uint32_t bytes_per_pixel;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t reserved_mask;
  } FramebufferInfo;

  /*
   * Normalized physical memory descriptor.
   * Extracted from UEFI memory map descriptors for early page frame allocation.
   */
  typedef struct
  {
    uint64_t address;
    uint64_t size;
    uint32_t type;
    uint32_t reserved;
  } MemoryMapEntry;

  /*
   * Normalized system memory map container.
   * descriptor_size preserves the firmware's physical descriptor stride.
   */
  typedef struct
  {
    MemoryMapEntry *entries;
    uint64_t entry_count;
    uint64_t descriptor_size;
    uint32_t descriptor_version;
    uint32_t reserved;
    uint64_t map_key;
    uint64_t total_memory;
    uint64_t free_memory;
  } MemoryMapInfo;

  /*
   * Root handoff structure passed to kernel_main().
   */
  typedef struct
  {
    FramebufferInfo framebuffer;
    MemoryMapInfo memory_map;
    uint64_t rsdp;
    uint64_t kernel_physical_base;
    uint64_t kernel_virtual_base;
    uint64_t kernel_size;
  } BootInfo;

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#define KERNEL_ABI __attribute__((sysv_abi))
#else
#define KERNEL_ABI
#endif

  typedef void(KERNEL_ABI *kernel_entry_t)(BootInfo *boot_info);

/*
 * Compile-time ABI assertions to guarantee identical structure layout
 * across both the bootloader (MS x64 ABI) and the kernel (System V ABI).
 */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define _BOOT_INFO_SA(c, m) static_assert(c, m)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define _BOOT_INFO_SA(c, m) _Static_assert(c, m)
#else
#define _BOOT_INFO_SA(c, m) extern char _boot_info_sa_dummy[(c) ? 1 : -1]
#endif

  _BOOT_INFO_SA(sizeof(FramebufferInfo) == 40, "FramebufferInfo size mismatch");
  _BOOT_INFO_SA(sizeof(MemoryMapEntry) == 24, "MemoryMapEntry size mismatch");
  _BOOT_INFO_SA(sizeof(MemoryMapInfo) == 56, "MemoryMapInfo size mismatch");
  _BOOT_INFO_SA(sizeof(BootInfo) == 128, "BootInfo size mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, framebuffer) == 0, "BootInfo.framebuffer offset mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, memory_map) == 40, "BootInfo.memory_map offset mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, rsdp) == 96, "BootInfo.rsdp offset mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, kernel_physical_base) == 104, "BootInfo.kernel_physical_base offset mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, kernel_virtual_base) == 112, "BootInfo.kernel_virtual_base offset mismatch");
  _BOOT_INFO_SA(offsetof(BootInfo, kernel_size) == 120, "BootInfo.kernel_size offset mismatch");

#ifdef __cplusplus
}
#endif

#endif /* BOOT_INFO_H */
