/*
 * protocols/gop.h - Graphics Output Protocol (GOP).
 *
 * Conforms to UEFI Specification 2.11 (Section 12.9).
 */

#ifndef _EFI_PROTOCOLS_GOP_H_
#define _EFI_PROTOCOLS_GOP_H_

#include "../efi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}

typedef struct
{
  UINT32 RedMask;
  UINT32 GreenMask;
  UINT32 BlueMask;
  UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum
{
  PixelRedGreenBlueReserved8BitPerColor = 0,
  PixelBlueGreenRedReserved8BitPerColor = 1,
  PixelBitMask = 2,
  PixelBltOnly = 3,
  PixelFormatMax = 4
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct
{
  UINT32 Version;
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  EFI_PIXEL_BITMASK PixelInformation;
  UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct
{
  UINT32 MaxMode;
  UINT32 Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo;
  EFI_PHYSICAL_ADDRESS FrameBufferBase;
  UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct
{
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef enum
{
  EfiBltVideoFill = 0,
  EfiBltVideoToBltBuffer = 1,
  EfiBltBufferToVideo = 2,
  EfiBltVideoToVideo = 3,
  EfiGraphicsOutputBltOperationMax = 4
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

/*
 * ABI assertions: UEFI enums are strictly 32-bit and structure layouts
 * must match the specification binary layout.
 */
_EFI_SA(sizeof(EFI_GRAPHICS_PIXEL_FORMAT) == 4, "EFI_GRAPHICS_PIXEL_FORMAT must be 32-bit");
_EFI_SA(sizeof(EFI_GRAPHICS_OUTPUT_BLT_OPERATION) == 4, "EFI_GRAPHICS_OUTPUT_BLT_OPERATION must be 32-bit");
_EFI_SA(sizeof(EFI_PIXEL_BITMASK) == 16, "EFI_PIXEL_BITMASK size mismatch");
_EFI_SA(sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) == 36, "EFI_GRAPHICS_OUTPUT_MODE_INFORMATION size mismatch");
_EFI_SA(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) == 4, "EFI_GRAPHICS_OUTPUT_BLT_PIXEL size mismatch");

typedef EFI_STATUS(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);

typedef EFI_STATUS(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN UINT32 ModeNumber);

typedef EFI_STATUS(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT)(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer OPTIONAL,
    IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX,
    IN UINTN SourceY,
    IN UINTN DestinationX,
    IN UINTN DestinationY,
    IN UINTN Width,
    IN UINTN Height,
    IN UINTN Delta OPTIONAL);

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE SetMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
};

#ifdef __cplusplus
}
#endif

#endif /* _EFI_PROTOCOLS_GOP_H_ */
