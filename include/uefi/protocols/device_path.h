/*
 * protocols/device_path.h - Device Path Protocol and node structures.
 *
 * Conforms to UEFI Specification 2.11 (Chapter 10).
 */

#ifndef _EFI_PROTOCOLS_DEVICE_PATH_H_
#define _EFI_PROTOCOLS_DEVICE_PATH_H_

#include "../efi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
  {0x09576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID \
  {0x0379be4e, 0xd706, 0x437d, {0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7, 0x72, 0xa4}}

/*
 * Device path structures are variable-length byte streams aligned to 1 byte.
 */
#pragma pack(push, 1)

struct _EFI_DEVICE_PATH_PROTOCOL
{
  UINT8 Type;
  UINT8 SubType;
  UINT8 Length[2];
};

#define EFI_DP_NODE_LENGTH(Dp) \
  ((UINT16)(((Dp)->Length[0]) | (((Dp)->Length[1]) << 8)))

#define EFI_DP_IS_VALID_NODE(Dp) \
  (EFI_DP_NODE_LENGTH(Dp) >= sizeof(EFI_DEVICE_PATH_PROTOCOL))

#define EFI_DP_SET_NODE_LENGTH(Dp, Len)             \
  do                                                \
  {                                                 \
    (Dp)->Length[0] = (UINT8)((Len) & 0xFF);        \
    (Dp)->Length[1] = (UINT8)(((Len) >> 8) & 0xFF); \
  } while (0)

#define EFI_NEXT_DEVICE_PATH_NODE(Dp) \
  ((EFI_DEVICE_PATH_PROTOCOL *)(((UINT8 *)(Dp)) + EFI_DP_NODE_LENGTH(Dp)))

/* Major Device Path Types */
#define HARDWARE_DEVICE_PATH 0x01
#define ACPI_DEVICE_PATH 0x02
#define MESSAGING_DEVICE_PATH 0x03
#define MEDIA_DEVICE_PATH 0x04
#define BBS_DEVICE_PATH 0x05
#define END_DEVICE_PATH_TYPE 0x7F

/* End of Device Path Nodes (Section 10.3.1) */
#define END_ENTIRE_DEVICE_PATH_SUBTYPE 0xFF
#define END_INSTANCE_DEVICE_PATH_SUBTYPE 0x01

#define EFI_IS_END_DEVICE_PATH(Dp) \
  ((Dp)->Type == END_DEVICE_PATH_TYPE)

#define EFI_IS_END_ENTIRE_DEVICE_PATH(Dp) \
  (((Dp)->Type == END_DEVICE_PATH_TYPE) && ((Dp)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE))

#define EFI_IS_END_INSTANCE_DEVICE_PATH(Dp) \
  (((Dp)->Type == END_DEVICE_PATH_TYPE) && ((Dp)->SubType == END_INSTANCE_DEVICE_PATH_SUBTYPE))

/* Hardware Device Path Nodes (Section 10.3.2) */
#define HW_PCI_DP 0x01
#define HW_PCCARD_DP 0x02
#define HW_MEMMAP_DP 0x03
#define HW_VENDOR_DP 0x04
#define HW_CONTROLLER_DP 0x05
#define HW_BMC_DP 0x06

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT8 Function;
  UINT8 Device;
} PCI_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT8 FunctionNumber;
} PCCARD_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT8 InterfaceType;
  UINT8 BaseAddress[8];
} BMC_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 MemoryType;
  EFI_PHYSICAL_ADDRESS StartingAddress;
  EFI_PHYSICAL_ADDRESS EndingAddress;
} MEMMAP_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_GUID Guid;
} VENDOR_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 ControllerNumber;
} CONTROLLER_DEVICE_PATH;

/* ACPI Device Path Nodes (Section 10.3.3) */
#define ACPI_DP 0x01
#define ACPI_EXTENDED_DP 0x02
#define ACPI_ADR_DP 0x03
#define ACPI_NVDIMM_DP 0x04

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 HID;
  UINT32 UID;
} ACPI_HID_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 HID;
  UINT32 UID;
  UINT32 CID;
} ACPI_EXTENDED_HID_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 ADR;
} ACPI_ADR_DEVICE_PATH;

/* Messaging Device Path Nodes (Section 10.3.4) */
#define MSG_ATAPI_DP 0x01
#define MSG_SCSI_DP 0x02
#define MSG_FIBRECHANNEL_DP 0x03
#define MSG_1394_DP 0x04
#define MSG_USB_DP 0x05
#define MSG_I2O_DP 0x06
#define MSG_INFINIBAND_DP 0x09
#define MSG_VENDOR_DP 0x0A
#define MSG_MAC_ADDR_DP 0x0B
#define MSG_IPv4_DP 0x0C
#define MSG_IPv6_DP 0x0D
#define MSG_UART_DP 0x0E
#define MSG_USB_CLASS_DP 0x0F
#define MSG_USB_WWID_DP 0x10
#define MSG_DEVICE_LOGICAL_UNIT_DP 0x11
#define MSG_SATA_DP 0x12
#define MSG_ISCSI_DP 0x13
#define MSG_VLAN_DP 0x14
#define MSG_FIBRECHANNELEX_DP 0x15
#define MSG_SASEX_DP 0x16
#define MSG_NVME_NAMESPACE_DP 0x17
#define MSG_URI_DP 0x18
#define MSG_UFS_DP 0x19
#define MSG_SD_DP 0x1A
#define MSG_BLUETOOTH_DP 0x1B
#define MSG_WIFI_DP 0x1C
#define MSG_EMMC_DP 0x1D
#define MSG_BLUETOOTH_LE_DP 0x1E
#define MSG_DNS_DP 0x1F
#define MSG_NVDIMM_NAMESPACE_DP 0x20
#define MSG_REST_SERVICE_DP 0x21
#define MSG_NVME_OF_NAMESPACE_DP 0x22

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT8 PrimarySecondary;
  UINT8 SlaveMaster;
  UINT16 Lun;
} ATAPI_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT16 Pun;
  UINT16 Lun;
} SCSI_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT16 VlanId;
} VLAN_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  CHAR8 Uri[1];
} URI_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT8 ParentPortNumber;
  UINT8 InterfaceNumber;
} USB_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT16 HBAPortNumber;
  UINT16 PortMultiplierPortNumber;
  UINT16 Lun;
} SATA_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 NamespaceId;
  UINT64 IEEEExtendedUniqueIdentifier;
} NVME_NAMESPACE_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_MAC_ADDRESS MacAddress;
  UINT8 IfType;
} MAC_ADDR_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_IPv4_ADDRESS LocalIpAddress;
  EFI_IPv4_ADDRESS RemoteIpAddress;
  UINT16 LocalPort;
  UINT16 RemotePort;
  UINT16 Protocol;
  BOOLEAN StaticIpAddress;
  EFI_IPv4_ADDRESS GatewayIpAddress;
  EFI_IPv4_ADDRESS SubnetMask;
} IPv4_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_IPv6_ADDRESS LocalIpAddress;
  EFI_IPv6_ADDRESS RemoteIpAddress;
  UINT16 LocalPort;
  UINT16 RemotePort;
  UINT16 Protocol;
  UINT8 IPAddressOrigin;
  UINT8 PrefixLength;
  EFI_IPv6_ADDRESS GatewayIpAddress;
} IPv6_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 Reserved;
  UINT64 BaudRate;
  UINT8 DataBits;
  UINT8 Parity;
  UINT8 StopBits;
} UART_DEVICE_PATH;

/* Media Device Path Nodes (Section 10.3.5) */
#define MEDIA_HARDDRIVE_DP 0x01
#define MEDIA_CDROM_DP 0x02
#define MEDIA_VENDOR_DP 0x03
#define MEDIA_FILEPATH_DP 0x04
#define MEDIA_PROTOCOL_DP 0x05
#define MEDIA_PIWG_FW_FILE_DP 0x06
#define MEDIA_PIWG_FW_VOL_DP 0x07
#define MEDIA_RELATIVE_OFFSET_RANGE_DP 0x08
#define MEDIA_RAM_DISK_DP 0x09

#define SIGNATURE_TYPE_NO_SIGNATURE 0x00
#define SIGNATURE_TYPE_MBR 0x01
#define SIGNATURE_TYPE_GUID 0x02

#define MBR_TYPE_PCAT 0x01
#define MBR_TYPE_EFI_PARTITION_TABLE_HEADER 0x02

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 PartitionNumber;
  UINT64 PartitionStart;
  UINT64 PartitionSize;
  UINT8 Signature[16];
  UINT8 MBRType;
  UINT8 SignatureType;
} HARDDRIVE_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 BootEntry;
  UINT64 PartitionStart;
  UINT64 PartitionSize;
} CDROM_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_GUID Protocol;
} MEDIA_PROTOCOL_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_GUID FvFileName;
} MEDIA_FW_VOL_FILEPATH_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_GUID FvName;
} MEDIA_FW_VOL_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT32 Reserved;
  UINT64 StartingOffset;
  UINT64 EndingOffset;
} MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  CHAR16 PathName[1];
} FILEPATH_DEVICE_PATH;

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_PHYSICAL_ADDRESS StartingAddress;
  EFI_PHYSICAL_ADDRESS EndingAddress;
  EFI_GUID TypeGuid;
  UINT16 Instance;
} RAM_DISK_DEVICE_PATH;

/* BIOS Boot Specification (BBS) Device Path (Section 10.3.6) */
#define BBS_DEVICE_PATH_SUBTYPE 0x01

typedef struct
{
  EFI_DEVICE_PATH_PROTOCOL Header;
  UINT16 DeviceType;
  UINT16 StatusFlag;
  CHAR8 String[1];
} BBS_BBS_DEVICE_PATH;

#pragma pack(pop)

/*
 * Device Path Utilities Protocol (Section 10.5).
 */
typedef struct _EFI_DEVICE_PATH_UTILITIES_PROTOCOL EFI_DEVICE_PATH_UTILITIES_PROTOCOL;

typedef UINTN(EFIAPI *EFI_DEVICE_PATH_UTILS_GET_DEVICE_PATH_SIZE)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_DUP_DEVICE_PATH)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_PATH)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *Src1,
    IN CONST EFI_DEVICE_PATH_PROTOCOL *Src2);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_DEVICE_NODE)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DeviceNode);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_APPEND_INSTANCE)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePathInstance);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_GET_NEXT_INSTANCE)(
    IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePathInstance,
    OUT UINTN *DevicePathInstanceSize);

typedef BOOLEAN(EFIAPI *EFI_DEVICE_PATH_UTILS_IS_MULTI_INSTANCE)(
    IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath);

typedef EFI_DEVICE_PATH_PROTOCOL *(EFIAPI *EFI_DEVICE_PATH_UTILS_CREATE_NODE)(
    IN UINT8 NodeType,
    IN UINT8 NodeSubType,
    IN UINT16 NodeLength);

struct _EFI_DEVICE_PATH_UTILITIES_PROTOCOL
{
  EFI_DEVICE_PATH_UTILS_GET_DEVICE_PATH_SIZE GetDevicePathSize;
  EFI_DEVICE_PATH_UTILS_DUP_DEVICE_PATH DuplicateDevicePath;
  EFI_DEVICE_PATH_UTILS_APPEND_PATH AppendDevicePath;
  EFI_DEVICE_PATH_UTILS_APPEND_DEVICE_NODE AppendDeviceNode;
  EFI_DEVICE_PATH_UTILS_APPEND_INSTANCE AppendDevicePathInstance;
  EFI_DEVICE_PATH_UTILS_GET_NEXT_INSTANCE GetNextDevicePathInstance;
  EFI_DEVICE_PATH_UTILS_IS_MULTI_INSTANCE IsDevicePathMultiInstance;
  EFI_DEVICE_PATH_UTILS_CREATE_NODE CreateDeviceNode;
};

#ifdef __cplusplus
}
#endif

#endif /* _EFI_PROTOCOLS_DEVICE_PATH_H_ */
