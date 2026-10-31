/*
 * efi_types.h - Core UEFI data types, constants, and status codes.
 *
 * Conforms to UEFI Specification 2.11.
 */

#ifndef _EFI_TYPES_H_
#define _EFI_TYPES_H_

#if defined(__UINT8_TYPE__) && defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0)
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
#else
#include <stdint.h>
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Calling conventions and argument modifiers (UEFI 2.11 Section 2.3).
 */
#ifndef EFIAPI
#if defined(_MSC_VER) || defined(_WIN32) || defined(__CYGWIN__)
#define EFIAPI __cdecl
#elif defined(__x86_64__) || defined(__amd64__)
#define EFIAPI __attribute__((ms_abi))
#else
#define EFIAPI
#endif
#endif

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef VOID
#define VOID void
#endif

  /*
   * Fundamental data types (UEFI 2.11 Section 2.3.1, Table 2-1).
   */
  typedef uint8_t BOOLEAN;

#ifndef TRUE
#define TRUE ((BOOLEAN)1)
#endif
#ifndef FALSE
#define FALSE ((BOOLEAN)0)
#endif
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((VOID *)0)
#endif
#endif

  typedef int8_t INT8;
  typedef int16_t INT16;
  typedef int32_t INT32;
  typedef int64_t INT64;

  typedef uint8_t UINT8;
  typedef uint16_t UINT16;
  typedef uint32_t UINT32;
  typedef uint64_t UINT64;

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || \
    defined(__aarch64__) || defined(_M_ARM64) ||                    \
    (defined(__riscv) && (__riscv_xlen == 64)) ||                   \
    (defined(__loongarch__) && (__loongarch_grlen == 64))
  typedef int64_t INTN;
  typedef uint64_t UINTN;
#define EFI_MAX_BIT (0x8000000000000000ULL)
#else
typedef int32_t INTN;
typedef uint32_t UINTN;
#define EFI_MAX_BIT (0x80000000UL)
#endif

#if defined(__SIZEOF_INT128__)
  __extension__ typedef __int128 INT128;
  __extension__ typedef unsigned __int128 UINT128;
#else
typedef struct
{
  UINT64 Low;
  INT64 High;
} INT128;

typedef struct
{
  UINT64 Low;
  UINT64 High;
} UINT128;
#endif

  typedef char CHAR8;
  typedef uint16_t CHAR16;

  typedef UINTN EFI_STATUS;
  typedef VOID *EFI_HANDLE;
  typedef VOID *EFI_EVENT;
  typedef UINT64 EFI_LBA;
  typedef UINTN EFI_TPL;
  typedef UINT64 EFI_PHYSICAL_ADDRESS;
  typedef UINT64 EFI_VIRTUAL_ADDRESS;

  typedef EFI_STATUS(EFIAPI *EFI_IMAGE_UNLOAD)(IN EFI_HANDLE ImageHandle);

  typedef struct _EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL;
  typedef struct _EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH_PROTOCOL;
  typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
  typedef struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL;
  typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

  /*
   * GUID and network address types (UEFI 2.11 Table 2-1).
   */
  typedef struct
  {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8];
  } EFI_GUID;

  typedef struct
  {
    UINT8 Addr[32];
  } EFI_MAC_ADDRESS;

  typedef struct
  {
    UINT8 Addr[4];
  } EFI_IPv4_ADDRESS;

  typedef struct
  {
    UINT8 Addr[16];
  } EFI_IPv6_ADDRESS;

  typedef union
  {
    UINT32 Addr[4];
    EFI_IPv4_ADDRESS v4;
    EFI_IPv6_ADDRESS v6;
  } EFI_IP_ADDRESS;

/*
 * UEFI status codes (UEFI 2.11 Appendix D).
 */
#define EFI_ENCODE_ERROR(Code) ((EFI_STATUS)(EFI_MAX_BIT | (Code)))
#define EFI_ENCODE_WARNING(Code) ((EFI_STATUS)(Code))
#define EFI_ERROR(StatusCode) (((INTN)(EFI_STATUS)(StatusCode)) < 0)

#define EFI_SUCCESS ((EFI_STATUS)0)

#define EFI_LOAD_ERROR EFI_ENCODE_ERROR(1)
#define EFI_INVALID_PARAMETER EFI_ENCODE_ERROR(2)
#define EFI_UNSUPPORTED EFI_ENCODE_ERROR(3)
#define EFI_BAD_BUFFER_SIZE EFI_ENCODE_ERROR(4)
#define EFI_BUFFER_TOO_SMALL EFI_ENCODE_ERROR(5)
#define EFI_NOT_READY EFI_ENCODE_ERROR(6)
#define EFI_DEVICE_ERROR EFI_ENCODE_ERROR(7)
#define EFI_WRITE_PROTECTED EFI_ENCODE_ERROR(8)
#define EFI_OUT_OF_RESOURCES EFI_ENCODE_ERROR(9)
#define EFI_VOLUME_CORRUPTED EFI_ENCODE_ERROR(10)
#define EFI_VOLUME_FULL EFI_ENCODE_ERROR(11)
#define EFI_NO_MEDIA EFI_ENCODE_ERROR(12)
#define EFI_MEDIA_CHANGED EFI_ENCODE_ERROR(13)
#define EFI_NOT_FOUND EFI_ENCODE_ERROR(14)
#define EFI_ACCESS_DENIED EFI_ENCODE_ERROR(15)
#define EFI_NO_RESPONSE EFI_ENCODE_ERROR(16)
#define EFI_NO_MAPPING EFI_ENCODE_ERROR(17)
#define EFI_TIMEOUT EFI_ENCODE_ERROR(18)
#define EFI_NOT_STARTED EFI_ENCODE_ERROR(19)
#define EFI_ALREADY_STARTED EFI_ENCODE_ERROR(20)
#define EFI_ABORTED EFI_ENCODE_ERROR(21)
#define EFI_ICMP_ERROR EFI_ENCODE_ERROR(22)
#define EFI_TFTP_ERROR EFI_ENCODE_ERROR(23)
#define EFI_PROTOCOL_ERROR EFI_ENCODE_ERROR(24)
#define EFI_INCOMPATIBLE_VERSION EFI_ENCODE_ERROR(25)
#define EFI_SECURITY_VIOLATION EFI_ENCODE_ERROR(26)
#define EFI_CRC_ERROR EFI_ENCODE_ERROR(27)
#define EFI_END_OF_MEDIA EFI_ENCODE_ERROR(28)
#define EFI_END_OF_FILE EFI_ENCODE_ERROR(31)
#define EFI_INVALID_LANGUAGE EFI_ENCODE_ERROR(32)
#define EFI_COMPROMISED_DATA EFI_ENCODE_ERROR(33)
#define EFI_IP_ADDRESS_CONFLICT EFI_ENCODE_ERROR(34)
#define EFI_HTTP_ERROR EFI_ENCODE_ERROR(35)

#define EFI_WARN_UNKNOWN_GLYPH EFI_ENCODE_WARNING(1)
#define EFI_WARN_DELETE_FAILURE EFI_ENCODE_WARNING(2)
#define EFI_WARN_WRITE_FAILURE EFI_ENCODE_WARNING(3)
#define EFI_WARN_BUFFER_TOO_SMALL EFI_ENCODE_WARNING(4)
#define EFI_WARN_STALE_DATA EFI_ENCODE_WARNING(5)
#define EFI_WARN_FILE_SYSTEM EFI_ENCODE_WARNING(6)
#define EFI_WARN_RESET_REQUIRED EFI_ENCODE_WARNING(7)

  /*
   * Standard table header (UEFI 2.11 Section 4.2).
   */
  typedef struct
  {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
  } EFI_TABLE_HEADER;

/*
 * Events, timers, and task priority levels (UEFI 2.11 Section 7.1).
 */
#define EVT_TIMER 0x80000000U
#define EVT_RUNTIME 0x40000000U
#define EVT_NOTIFY_WAIT 0x00000100U
#define EVT_NOTIFY_SIGNAL 0x00000200U
#define EVT_SIGNAL_EXIT_BOOT_SERVICES 0x00000201U
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 0x60000202U

#define EFI_EVENT_GROUP_EXIT_BOOT_SERVICES \
  {0x27abf055, 0xb1b8, 0x4c26, {0x80, 0x48, 0x74, 0x8f, 0x37, 0xba, 0xa2, 0xdf}}

#define EFI_EVENT_GROUP_BEFORE_EXIT_BOOT_SERVICES \
  {0x8be0e274, 0x3970, 0x4b44, {0x80, 0xc5, 0x1a, 0xb9, 0x50, 0x2f, 0x3b, 0xfc}}

#define EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE \
  {0x13fa7698, 0xc831, 0x49c7, {0x87, 0xea, 0x8f, 0x43, 0xfc, 0xc2, 0x51, 0x96}}

#define EFI_EVENT_GROUP_MEMORY_MAP_CHANGE \
  {0x78bee926, 0x692f, 0x48fd, {0x9e, 0xdb, 0x01, 0x42, 0x2e, 0xf0, 0xd7, 0xab}}

#define EFI_EVENT_GROUP_READY_TO_BOOT \
  {0x7ce88fb3, 0x4bd7, 0x4679, {0x87, 0xa8, 0xa8, 0xd8, 0xde, 0xe5, 0x0d, 0x2b}}

#define EFI_EVENT_GROUP_AFTER_READY_TO_BOOT \
  {0x3a2a00ad, 0x98b9, 0x4cdf, {0xa4, 0x78, 0x70, 0x27, 0x77, 0xf1, 0xc1, 0x0b}}

#define EFI_EVENT_GROUP_RESET_SYSTEM \
  {0x62da6a56, 0x13fb, 0x485a, {0xa8, 0xda, 0xa3, 0xdd, 0x79, 0x12, 0xcb, 0x6b}}

#define TPL_APPLICATION 4
#define TPL_CALLBACK 8
#define TPL_NOTIFY 16
#define TPL_HIGH_LEVEL 31

  typedef enum
  {
    TimerCancel,
    TimerPeriodic,
    TimerRelative
  } EFI_TIMER_DELAY;

/*
 * Memory allocation and descriptor definitions (UEFI 2.11 Section 7.2).
 */
#define EFI_PAGE_SIZE 4096
#define EFI_PAGE_MASK 0xFFF
#define EFI_PAGE_SHIFT 12

#define EFI_SIZE_TO_PAGES(Size) \
  (((UINTN)(Size) >> EFI_PAGE_SHIFT) + ((((UINTN)(Size) & EFI_PAGE_MASK) != 0) ? 1 : 0))

  typedef enum
  {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
  } EFI_ALLOCATE_TYPE;

  typedef enum
  {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
  } EFI_MEMORY_TYPE;

#define EFI_MEMORY_UC 0x0000000000000001ULL
#define EFI_MEMORY_WC 0x0000000000000002ULL
#define EFI_MEMORY_WT 0x0000000000000004ULL
#define EFI_MEMORY_WB 0x0000000000000008ULL
#define EFI_MEMORY_UCE 0x0000000000000010ULL
#define EFI_MEMORY_WP 0x0000000000001000ULL
#define EFI_MEMORY_RP 0x0000000000002000ULL
#define EFI_MEMORY_XP 0x0000000000004000ULL
#define EFI_MEMORY_NV 0x0000000000008000ULL
#define EFI_MEMORY_MORE_RELIABLE 0x0000000000010000ULL
#define EFI_MEMORY_RO 0x0000000000020000ULL
#define EFI_MEMORY_SP 0x0000000000040000ULL
#define EFI_MEMORY_CPU_CRYPTO 0x0000000000080000ULL
#define EFI_MEMORY_HOT_PLUGGABLE 0x0000000000100000ULL
#define EFI_MEMORY_RUNTIME 0x8000000000000000ULL
#define EFI_MEMORY_ISA_VALID 0x4000000000000000ULL
#define EFI_MEMORY_ISA_MASK 0x0FFFF00000000000ULL

#define EFI_MEMORY_DESCRIPTOR_VERSION 1

  typedef struct
  {
    UINT32 Type;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
  } EFI_MEMORY_DESCRIPTOR;

  /*
   * Protocol handlers and search types (UEFI 2.11 Section 7.3).
   */
  typedef enum
  {
    EFI_NATIVE_INTERFACE
  } EFI_INTERFACE_TYPE;

  typedef enum
  {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
  } EFI_LOCATE_SEARCH_TYPE;

  typedef struct
  {
    EFI_HANDLE AgentHandle;
    EFI_HANDLE ControllerHandle;
    UINT32 Attributes;
    UINT32 OpenCount;
  } EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL 0x00000001U
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL 0x00000002U
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL 0x00000004U
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008U
#define EFI_OPEN_PROTOCOL_BY_DRIVER 0x00000010U
#define EFI_OPEN_PROTOCOL_EXCLUSIVE 0x00000020U

/*
 * Runtime services types: time, variables, system reset (UEFI 2.11 Chapter 8).
 */
#define EFI_VARIABLE_NON_VOLATILE 0x00000001U
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002U
#define EFI_VARIABLE_RUNTIME_ACCESS 0x00000004U
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD 0x00000008U
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x00000010U
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020U
#define EFI_VARIABLE_APPEND_WRITE 0x00000040U
#define EFI_VARIABLE_ENHANCED_AUTHENTICATED_ACCESS 0x00000080U

#define EFI_UNSPECIFIED_TIMEZONE 0x07FF
#define EFI_TIME_ADJUST_DAYLIGHT 0x01
#define EFI_TIME_IN_DAYLIGHT 0x02

  typedef struct
  {
    UINT16 Year;
    UINT8 Month;
    UINT8 Day;
    UINT8 Hour;
    UINT8 Minute;
    UINT8 Second;
    UINT8 Pad1;
    UINT32 Nanosecond;
    INT16 TimeZone;
    UINT8 Daylight;
    UINT8 Pad2;
  } EFI_TIME;

  typedef struct
  {
    UINT32 Resolution;
    UINT32 Accuracy;
    BOOLEAN SetsToZero;
  } EFI_TIME_CAPABILITIES;

#define EFI_OPTIONAL_PTR 0x00000001U

  typedef enum
  {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
  } EFI_RESET_TYPE;

  typedef struct
  {
    UINT64 Length;
    union
    {
      EFI_PHYSICAL_ADDRESS DataBlock;
      EFI_PHYSICAL_ADDRESS ContinuationPointer;
    } Union;
  } EFI_CAPSULE_BLOCK_DESCRIPTOR;

  typedef struct
  {
    EFI_GUID CapsuleGuid;
    UINT32 HeaderSize;
    UINT32 Flags;
    UINT32 CapsuleImageSize;
  } EFI_CAPSULE_HEADER;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET 0x00010000U
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE 0x00020000U
#define CAPSULE_FLAGS_INITIATE_RESET 0x00040000U

/*
 * Static assertions to verify ABI structure size invariants.
 */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define _EFI_SA(c, m) static_assert(c, m)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define _EFI_SA(c, m) _Static_assert(c, m)
#else
#define _EFI_SA(c, m) extern char _efi_static_assert_dummy[(c) ? 1 : -1]
#endif
  _EFI_SA(sizeof(BOOLEAN) == 1, "BOOLEAN must be 1 byte");
  _EFI_SA(sizeof(CHAR16) == 2, "CHAR16 must be 2 bytes");
  _EFI_SA(sizeof(EFI_GUID) == 16, "EFI_GUID size mismatch");
  _EFI_SA(sizeof(EFI_MAC_ADDRESS) == 32, "EFI_MAC_ADDRESS size mismatch");
  _EFI_SA(sizeof(EFI_TIME) == 16, "EFI_TIME size mismatch");
  _EFI_SA(sizeof(EFI_TABLE_HEADER) == 24, "EFI_TABLE_HEADER size mismatch");
  _EFI_SA(sizeof(EFI_CAPSULE_HEADER) == 28, "EFI_CAPSULE_HEADER size mismatch");

#ifdef __cplusplus
}
#endif

#endif /* _EFI_TYPES_H_ */
