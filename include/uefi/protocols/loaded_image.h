/*
 * protocols/loaded_image.h - Loaded Image and Image Device Path Protocols.
 *
 * Conforms to UEFI Specification 2.11 (Chapter 9).
 */

#ifndef _EFI_PROTOCOLS_LOADED_IMAGE_H_
#define _EFI_PROTOCOLS_LOADED_IMAGE_H_

#include "../efi_types.h"
#include "device_path.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* Forward declaration for system table without typedef collision */
  struct _EFI_SYSTEM_TABLE;

/*
 * Loaded Image Protocol (UEFI 2.11 Section 9.1).
 */
#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
  {0x5b1b31a1, 0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000U

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

#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 8)
  _EFI_SA(sizeof(EFI_LOADED_IMAGE_PROTOCOL) == 96, "EFI_LOADED_IMAGE_PROTOCOL size mismatch (64-bit)");
  _EFI_SA(__builtin_offsetof(EFI_LOADED_IMAGE_PROTOCOL, ImageBase) == 64, "EFI_LOADED_IMAGE_PROTOCOL ImageBase offset mismatch (64-bit)");
#elif defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4)
_EFI_SA(__builtin_offsetof(EFI_LOADED_IMAGE_PROTOCOL, ImageBase) == 32, "EFI_LOADED_IMAGE_PROTOCOL ImageBase offset mismatch (32-bit)");
#endif

/*
 * Loaded Image Device Path Protocol (UEFI 2.11 Section 9.2).
 * Shares the exact structure layout as EFI_DEVICE_PATH_PROTOCOL.
 */
#define EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID \
  {0xbc62157e, 0x3e33, 0x4fec, {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf}}

  typedef EFI_DEVICE_PATH_PROTOCOL EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL;

#ifdef __cplusplus
}
#endif

#endif /* _EFI_PROTOCOLS_LOADED_IMAGE_H_ */
