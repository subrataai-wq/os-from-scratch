/*
 * protocols/media.h - File System, Block I/O, and Load File Protocols.
 *
 * Conforms to UEFI Specification 2.11 (Chapter 13).
 */

#ifndef _EFI_PROTOCOLS_MEDIA_H_
#define _EFI_PROTOCOLS_MEDIA_H_

#include "../efi_types.h"
#include "device_path.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_BLOCK_IO_PROTOCOL EFI_BLOCK_IO_PROTOCOL;
typedef struct _EFI_LOAD_FILE_PROTOCOL EFI_LOAD_FILE_PROTOCOL;
typedef struct _EFI_LOAD_FILE2_PROTOCOL EFI_LOAD_FILE2_PROTOCOL;

/*
 * File Mode and Attribute Constants (UEFI 2.11 Section 13.5).
 */
#define EFI_FILE_MODE_READ 0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE 0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL

#define EFI_FILE_READ_ONLY 0x0000000000000001ULL
#define EFI_FILE_HIDDEN 0x0000000000000002ULL
#define EFI_FILE_SYSTEM 0x0000000000000004ULL
#define EFI_FILE_RESERVED 0x0000000000000008ULL
#define EFI_FILE_DIRECTORY 0x0000000000000010ULL
#define EFI_FILE_ARCHIVE 0x0000000000000020ULL
#define EFI_FILE_VALID_ATTR 0x0000000000000037ULL

/*
 * File Information Structures (UEFI 2.11 Section 13.5.16 - 13.5.18).
 */
#define EFI_FILE_INFO_ID \
  {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_FILE_SYSTEM_INFO_ID \
  {0x09576e93, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_FILE_SYSTEM_VOLUME_LABEL_ID \
  {0xdb47d7d3, 0xfe81, 0x11d3, {0x9a, 0x35, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

typedef struct
{
  UINT64 Size;
  UINT64 FileSize;
  UINT64 PhysicalSize;
  EFI_TIME CreateTime;
  EFI_TIME LastAccessTime;
  EFI_TIME ModificationTime;
  UINT64 Attribute;
  CHAR16 FileName[1];
} EFI_FILE_INFO;

typedef struct
{
  UINT64 Size;
  BOOLEAN ReadOnly;
  UINT64 VolumeSize;
  UINT64 FreeSpace;
  UINT32 BlockSize;
  CHAR16 VolumeLabel[1];
} EFI_FILE_SYSTEM_INFO;

typedef struct
{
  CHAR16 VolumeLabel[1];
} EFI_FILE_SYSTEM_VOLUME_LABEL;

/*
 * EFI File Protocol (UEFI 2.11 Section 13.5).
 */
#define EFI_FILE_PROTOCOL_REVISION 0x00010000
#define EFI_FILE_PROTOCOL_REVISION2 0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

typedef EFI_STATUS(EFIAPI *EFI_FILE_OPEN)(
    IN EFI_FILE_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16 *FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes);

typedef EFI_STATUS(EFIAPI *EFI_FILE_CLOSE)(
    IN EFI_FILE_PROTOCOL *This);

typedef EFI_STATUS(EFIAPI *EFI_FILE_DELETE)(
    IN EFI_FILE_PROTOCOL *This);

typedef EFI_STATUS(EFIAPI *EFI_FILE_READ)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_FILE_WRITE)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN *BufferSize,
    IN VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_FILE_SET_POSITION)(
    IN EFI_FILE_PROTOCOL *This,
    IN UINT64 Position);

typedef EFI_STATUS(EFIAPI *EFI_FILE_GET_POSITION)(
    IN EFI_FILE_PROTOCOL *This,
    OUT UINT64 *Position);

typedef EFI_STATUS(EFIAPI *EFI_FILE_GET_INFO)(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID *InformationType,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_FILE_SET_INFO)(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID *InformationType,
    IN UINTN BufferSize,
    IN VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_FILE_FLUSH)(
    IN EFI_FILE_PROTOCOL *This);

typedef struct
{
  EFI_EVENT Event;
  EFI_STATUS Status;
  UINTN BufferSize;
  VOID *Buffer;
} EFI_FILE_IO_TOKEN;

typedef EFI_STATUS(EFIAPI *EFI_FILE_OPEN_EX)(
    IN EFI_FILE_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16 *FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes,
    IN OUT EFI_FILE_IO_TOKEN *Token);

typedef EFI_STATUS(EFIAPI *EFI_FILE_READ_EX)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token);

typedef EFI_STATUS(EFIAPI *EFI_FILE_WRITE_EX)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token);

typedef EFI_STATUS(EFIAPI *EFI_FILE_FLUSH_EX)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT EFI_FILE_IO_TOKEN *Token);

struct _EFI_FILE_PROTOCOL
{
  UINT64 Revision;
  EFI_FILE_OPEN Open;
  EFI_FILE_CLOSE Close;
  EFI_FILE_DELETE Delete;
  EFI_FILE_READ Read;
  EFI_FILE_WRITE Write;
  EFI_FILE_GET_POSITION GetPosition;
  EFI_FILE_SET_POSITION SetPosition;
  EFI_FILE_GET_INFO GetInfo;
  EFI_FILE_SET_INFO SetInfo;
  EFI_FILE_FLUSH Flush;
  EFI_FILE_OPEN_EX OpenEx;
  EFI_FILE_READ_EX ReadEx;
  EFI_FILE_WRITE_EX WriteEx;
  EFI_FILE_FLUSH_EX FlushEx;
};

/*
 * Simple File System Protocol (UEFI 2.11 Section 13.4).
 */
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
  {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000

typedef EFI_STATUS(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
    IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **Root);

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
{
  UINT64 Revision;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
};

/*
 * Block I/O Protocol (UEFI 2.11 Section 13.9).
 */
#define EFI_BLOCK_IO_PROTOCOL_GUID \
  {0x964e5b21, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_BLOCK_IO_PROTOCOL_REVISION 0x00010000
#define EFI_BLOCK_IO_PROTOCOL_REVISION2 0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3 0x0002001F

typedef struct
{
  UINT32 MediaId;
  BOOLEAN RemovableMedia;
  BOOLEAN MediaPresent;
  BOOLEAN LogicalPartition;
  BOOLEAN ReadOnly;
  BOOLEAN WriteCaching;
  UINT32 BlockSize;
  UINT32 IoAlign;
  EFI_LBA LastBlock;
  EFI_LBA LowestAlignedLba;
  UINT32 LogicalBlocksPerPhysicalBlock;
  UINT32 OptimalTransferLengthGranularity;
} EFI_BLOCK_IO_MEDIA;

typedef EFI_STATUS(EFIAPI *EFI_BLOCK_RESET)(
    IN EFI_BLOCK_IO_PROTOCOL *This,
    IN BOOLEAN ExtendedVerification);

typedef EFI_STATUS(EFIAPI *EFI_BLOCK_READ)(
    IN EFI_BLOCK_IO_PROTOCOL *This,
    IN UINT32 MediaId,
    IN EFI_LBA Lba,
    IN UINTN BufferSize,
    OUT VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_BLOCK_WRITE)(
    IN EFI_BLOCK_IO_PROTOCOL *This,
    IN UINT32 MediaId,
    IN EFI_LBA Lba,
    IN UINTN BufferSize,
    IN VOID *Buffer);

typedef EFI_STATUS(EFIAPI *EFI_BLOCK_FLUSH)(
    IN EFI_BLOCK_IO_PROTOCOL *This);

struct _EFI_BLOCK_IO_PROTOCOL
{
  UINT64 Revision;
  EFI_BLOCK_IO_MEDIA *Media;
  EFI_BLOCK_RESET Reset;
  EFI_BLOCK_READ ReadBlocks;
  EFI_BLOCK_WRITE WriteBlocks;
  EFI_BLOCK_FLUSH FlushBlocks;
};

/*
 * Load File and Load File 2 Protocols (UEFI 2.11 Section 13.1 & 13.2).
 */
#define EFI_LOAD_FILE_PROTOCOL_GUID \
  {0x56ec3091, 0x954c, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_LOAD_FILE2_PROTOCOL_GUID \
  {0x4006c0c1, 0xfcb3, 0x403e, {0x99, 0x6d, 0x4a, 0x6c, 0x87, 0x24, 0xe0, 0x6d}}

typedef EFI_STATUS(EFIAPI *EFI_LOAD_FILE)(
    IN EFI_LOAD_FILE_PROTOCOL *This,
    IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer OPTIONAL);

struct _EFI_LOAD_FILE_PROTOCOL
{
  EFI_LOAD_FILE LoadFile;
};

typedef EFI_STATUS(EFIAPI *EFI_LOAD_FILE2)(
    IN EFI_LOAD_FILE2_PROTOCOL *This,
    IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
    IN BOOLEAN BootPolicy,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer OPTIONAL);

struct _EFI_LOAD_FILE2_PROTOCOL
{
  EFI_LOAD_FILE2 LoadFile;
};

#ifdef __cplusplus
}
#endif

#endif /* _EFI_PROTOCOLS_MEDIA_H_ */
