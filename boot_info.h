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

#if defined(__has_include)
#  if __has_include("uefi/uefi.h")
#    include "uefi/uefi.h"
#  elif __has_include("include/uefi/uefi.h")
#    include "include/uefi/uefi.h"
#  endif
#endif

#ifndef _EFI_TYPES_H_
#include <stdint.h>
typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t   INT8;
typedef int16_t  INT16;
typedef int32_t  INT32;
typedef int64_t  INT64;
typedef uint64_t UINTN;
typedef void     VOID;
typedef void*    EFI_HANDLE;
#endif

/*
 * Complete forward declaration for EFI_LOADED_IMAGE_PROTOCOL from efi_types.h.
 * Conforms to UEFI 2.11 Section 9.1.
 */
#ifdef _EFI_TYPES_H_
struct _EFI_LOADED_IMAGE_PROTOCOL
{
  UINT32 Revision;
  EFI_HANDLE ParentHandle;
  struct _EFI_SYSTEM_TABLE *SystemTable;

  EFI_HANDLE DeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL *FilePath;
  VOID *Reserved;

  UINT32 LoadOptionsSize;
  VOID *LoadOptions;

  VOID *ImageBase;
  UINT64 ImageSize;
  EFI_MEMORY_TYPE ImageCodeType;
  EFI_MEMORY_TYPE ImageDataType;
  EFI_IMAGE_UNLOAD Unload;
};
#endif

/*
 * Linear framebuffer configuration acquired from Graphics Output Protocol (GOP).
 * pixels_per_scanline accounts for stride pitch padding required by display hardware.
 */
typedef struct
{
  UINT64 base_address;
  UINT32 width;
  UINT32 height;
  UINT32 pixels_per_scanline;
  UINT32 bytes_per_pixel;
  UINT32 red_mask;
  UINT32 green_mask;
  UINT32 blue_mask;
  UINT32 reserved_mask;
} FramebufferInfo;

/*
 * Normalized physical memory descriptor.
 * Extracted from UEFI memory map descriptors for early page frame allocation.
 */
typedef struct
{
  UINT64 address;
  UINT64 size;
  UINT32 type;
  UINT32 reserved;
} MemoryMapEntry;

typedef struct
{
  MemoryMapEntry *entries;
  UINT64 entry_count;
  UINT64 descriptor_size;
  UINT32 descriptor_version;
  UINT32 reserved;
  UINT64 map_key;
  UINT64 total_memory;
  UINT64 free_memory;
} MemoryMapInfo;

/*
 * Root handoff structure passed to kernel_main().
 */
typedef struct
{
  FramebufferInfo framebuffer;
  MemoryMapInfo memory_map;
  UINT64 rsdp;
  UINT64 kernel_physical_base;
  UINT64 kernel_virtual_base;
  UINT64 kernel_size;
} BootInfo;

#endif /* BOOT_INFO_H */
