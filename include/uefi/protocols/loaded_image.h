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
