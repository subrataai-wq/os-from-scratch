#ifndef EFI_H
#define EFI_H

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef long long INTN;
typedef UINT64 UINTN; /* x64 only */
typedef UINT8 BOOLEAN;
typedef __CHAR16_TYPE__ CHAR16;
typedef void VOID;
typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;
typedef VOID *EFI_EVENT;
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

#define IN
#define OUT
#define NULL ((void *)0)
#ifndef EFIAPI
#if defined(_MSC_VER)
// Microsoft Visual Studio uses the correct ABI by default on x64
// and doesn't support __attribute__
#define EFIAPI
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__x86_64__)
// GCC/Clang on Linux/macOS need to be forced to use the MS ABI
#define EFIAPI __attribute__((ms_abi))
#else
// For 32-bit x86, ARM, or AArch64, standard C convention is used
#define EFIAPI
#endif
#else
// Fallback for other compilers
#define EFIAPI
#endif
#endif

#define EFI_SUCCESS 0
#define EFI_ERR(n) ((EFI_STATUS)(0x8000000000000000ULL | (n)))
#define EFI_BUFFER_TOO_SMALL EFI_ERR(5) /* verify in the Status Codes appendix */
#define EFI_ERROR(s) (((INTN)(s)) < 0)

typedef struct
{
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8 Data4[8];
} EFI_GUID;

typedef struct
{
  UINT64 Signature;
  UINT32 Revision, HeaderSize, CRC32, Reserved;
} EFI_TABLE_HEADER;

/* ---- memory (§7.2) ---- */
#define AllocateAnyPages 0
#define EfiLoaderData 2
#define EfiConventionalMemory 7
typedef struct
{
  UINT32 Type;
  EFI_PHYSICAL_ADDRESS PhysicalStart;
  EFI_VIRTUAL_ADDRESS VirtualStart;
  UINT64 NumberOfPages;
  UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

/* ---- console (§12.3, §12.4) ---- */
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
  VOID *Reset;
  EFI_STATUS(EFIAPI *OutputString)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);
  VOID *TestString, *QueryMode, *SetMode, *SetAttribute;
  EFI_STATUS(EFIAPI *ClearScreen)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
  VOID *SetCursorPosition, *EnableCursor, *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct
{
  UINT16 ScanCode;
  CHAR16 UnicodeChar;
} EFI_INPUT_KEY;
typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL
{
  VOID *Reset;
  EFI_STATUS(EFIAPI *ReadKeyStroke)(struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);
  EFI_EVENT WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

/* ---- boot services (§4.4.1): order must match the spec exactly ---- */
typedef struct EFI_BOOT_SERVICES
{
  EFI_TABLE_HEADER Hdr;
  VOID *RaiseTPL, *RestoreTPL;
  EFI_STATUS(EFIAPI *AllocatePages)(UINT32 Type, UINT32 MemoryType, UINTN Pages, EFI_PHYSICAL_ADDRESS *Memory);
  VOID *FreePages;
  EFI_STATUS(EFIAPI *GetMemoryMap)(UINTN *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap,
                                   UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);
  EFI_STATUS(EFIAPI *AllocatePool)(UINT32 PoolType, UINTN Size, VOID **Buffer);
  EFI_STATUS(EFIAPI *FreePool)(VOID *Buffer);
  VOID *CreateEvent, *SetTimer;
  EFI_STATUS(EFIAPI *WaitForEvent)(UINTN NumberOfEvents, EFI_EVENT *Event, UINTN *Index);
  VOID *SignalEvent, *CloseEvent, *CheckEvent;
  VOID *InstallProtocolInterface, *ReinstallProtocolInterface, *UninstallProtocolInterface;
  EFI_STATUS(EFIAPI *HandleProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, VOID **Interface);
  VOID *Reserved, *RegisterProtocolNotify, *LocateHandle, *LocateDevicePath, *InstallConfigurationTable;
  VOID *LoadImage, *StartImage, *Exit, *UnloadImage;
  EFI_STATUS(EFIAPI *ExitBootServices)(EFI_HANDLE ImageHandle, UINTN MapKey);
  VOID *GetNextMonotonicCount;
  EFI_STATUS(EFIAPI *Stall)(UINTN Microseconds);
  EFI_STATUS(EFIAPI *SetWatchdogTimer)(UINTN Timeout, UINT64 WatchdogCode, UINTN DataSize, CHAR16 *WatchdogData);
  VOID *ConnectController, *DisconnectController, *OpenProtocol, *CloseProtocol,
      *OpenProtocolInformation, *ProtocolsPerHandle, *LocateHandleBuffer;
  EFI_STATUS(EFIAPI *LocateProtocol)(EFI_GUID *Protocol, VOID *Registration, VOID **Interface);
  VOID *InstallMultipleProtocolInterfaces, *UninstallMultipleProtocolInterfaces,
      *CalculateCrc32, *CopyMem, *SetMem, *CreateEventEx;
} EFI_BOOT_SERVICES;

/* ---- system table (§4.3.1) ---- */
typedef struct
{
  EFI_GUID VendorGuid;
  VOID *VendorTable;
} EFI_CONFIGURATION_TABLE;
typedef struct
{
  EFI_TABLE_HEADER Hdr;
  CHAR16 *FirmwareVendor;
  UINT32 FirmwareRevision;
  EFI_HANDLE ConsoleInHandle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
  EFI_HANDLE ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
  EFI_HANDLE StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
  VOID *RuntimeServices;
  EFI_BOOT_SERVICES *BootServices;
  UINTN NumberOfTableEntries;
  EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;

/* ---- loaded image (§9.1) ---- */
typedef struct
{
  UINT32 Revision;
  EFI_HANDLE ParentHandle;
  EFI_SYSTEM_TABLE *SystemTable;
  EFI_HANDLE DeviceHandle;
  VOID *FilePath;
  VOID *Reserved;
  UINT32 LoadOptionsSize;
  VOID *LoadOptions;
  VOID *ImageBase;
  UINT64 ImageSize;
  UINT32 ImageCodeType, ImageDataType;
  VOID *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

/* ---- files (§13.4, §13.5) ---- */
#define EFI_FILE_MODE_READ 0x1ULL
typedef struct EFI_FILE_PROTOCOL
{
  UINT64 Revision;
  EFI_STATUS(EFIAPI *Open)(struct EFI_FILE_PROTOCOL *This, struct EFI_FILE_PROTOCOL **NewHandle,
                           CHAR16 *FileName, UINT64 OpenMode, UINT64 Attributes);
  EFI_STATUS(EFIAPI *Close)(struct EFI_FILE_PROTOCOL *This);
  VOID *Delete;
  EFI_STATUS(EFIAPI *Read)(struct EFI_FILE_PROTOCOL *This, UINTN *BufferSize, VOID *Buffer);
  VOID *Write;
  EFI_STATUS(EFIAPI *GetPosition)(struct EFI_FILE_PROTOCOL *This, UINT64 *Position);
  EFI_STATUS(EFIAPI *SetPosition)(struct EFI_FILE_PROTOCOL *This, UINT64 Position);
  VOID *GetInfo, *SetInfo, *Flush, *OpenEx, *ReadEx, *WriteEx, *FlushEx;
} EFI_FILE_PROTOCOL;

typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
{
  UINT64 Revision;
  EFI_STATUS(EFIAPI *OpenVolume)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, EFI_FILE_PROTOCOL **Root);
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

/* ---- graphics output (§12.9) ---- */
typedef struct
{
  UINT32 RedMask, GreenMask, BlueMask, ReservedMask;
} EFI_PIXEL_BITMASK;
typedef struct
{
  UINT32 Version, HorizontalResolution, VerticalResolution;
  UINT32 PixelFormat; /* 0 = RGBR, 1 = BGRR, 2 = BitMask, 3 = BltOnly */
  EFI_PIXEL_BITMASK PixelInformation;
  UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;
typedef struct
{
  UINT32 MaxMode, Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo;
  EFI_PHYSICAL_ADDRESS FrameBufferBase;
  UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;
typedef struct
{
  VOID *QueryMode, *SetMode, *Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

/* ---- GUIDs (copied from the spec) ---- */
static EFI_GUID gEfiLoadedImageGuid = {0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
static EFI_GUID gEfiGraphicsOutputGuid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
static EFI_GUID gEfiSimpleFileSystemGuid = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
static EFI_GUID gEfiAcpi20Guid = {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

#endif