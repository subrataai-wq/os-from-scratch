/*
 * main.c - UEFI Bootloader & Kernel Handoff (Stage 1 & Stage 2)
 *
 * Responsibilities:
 *  1. Query firmware tables to build a unified BootInfo structure (memory map,
 *     ACPI 2.0+ RSDP, linear framebuffer geometry).
 *  2. Locate and parse the freestanding 64-bit ELF kernel image from the ESP.
 *  3. Allocate page-aligned physical memory for PT_LOAD segments and clear BSS.
 *  4. Terminate UEFI runtime boot services via ExitBootServices().
 *  5. Transfer execution to kernel_main() conforming to System V AMD64 ABI.
 */

#include "uefi/uefi.h"
#include "boot_info.h"

static const EFI_GUID g_acpi20_guid = EFI_ACPI_20_TABLE_GUID;
static const EFI_GUID g_acpi10_guid = EFI_ACPI_10_TABLE_GUID;
static const EFI_GUID g_loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const EFI_GUID g_simple_file_system_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static const EFI_GUID g_graphics_output_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static EFI_SYSTEM_TABLE *g_st = NULL;
static EFI_BOOT_SERVICES *g_bs = NULL;
static BootInfo g_boot_info;

#define ELF_MAGIC 0x464C457FU /* 0x7F, 'E', 'L', 'F' */
#define PT_LOAD   1U

typedef struct
{
  UINT8  e_ident[16];
  UINT16 e_type;
  UINT16 e_machine;
  UINT32 e_version;
  UINT64 e_entry;
  UINT64 e_phoff;
  UINT64 e_shoff;
  UINT32 e_flags;
  UINT16 e_ehsize;
  UINT16 e_phentsize;
  UINT16 e_phnum;
  UINT16 e_shentsize;
  UINT16 e_shnum;
  UINT16 e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
  UINT32 p_type;
  UINT32 p_flags;
  UINT64 p_offset;
  UINT64 p_vaddr;
  UINT64 p_paddr;
  UINT64 p_filesz;
  UINT64 p_memsz;
  UINT64 p_align;
} Elf64_Phdr;

typedef void (*kernel_entry_t)(BootInfo *boot_info);

/*
 * Low-level UEFI console output helpers.
 */

static void print(const CHAR16 *string)
{
  if (g_st && g_st->ConOut)
  {
    g_st->ConOut->OutputString(g_st->ConOut, (CHAR16 *)string);
  }
}

static void set_color(UINTN foreground, UINTN background)
{
  if (g_st && g_st->ConOut)
  {
    g_st->ConOut->SetAttribute(g_st->ConOut, EFI_TEXT_ATTR(foreground, background));
  }
}

/* Visual indentation prevents glyph clipping against OVMF display boundaries */
static void print_status_tag(const CHAR16 *tag, UINTN color)
{
  print(u"  [");
  set_color(color, EFI_BACKGROUND_BLACK);
  print(tag);
  set_color(EFI_LIGHTGRAY, EFI_BACKGROUND_BLACK);
  print(u"] ");
}

static void print_hex(UINT64 value)
{
  CHAR16 buffer[19];
  buffer[0] = u'0';
  buffer[1] = u'x';
  for (int i = 0; i < 16; i++)
  {
    UINT8 nibble = (UINT8)((value >> (60 - 4 * i)) & 0x0F);
    buffer[2 + i] = (nibble < 10) ? (u'0' + nibble) : (u'A' + (nibble - 10));
  }
  buffer[18] = 0;
  print(buffer);
}

static void print_dec(UINT64 value)
{
  CHAR16 buffer[21];
  int i = 20;
  buffer[i] = 0;
  do
  {
    buffer[--i] = u'0' + (CHAR16)(value % 10);
    value /= 10;
  } while (value > 0);
  print(&buffer[i]);
}

static BOOLEAN guid_equal(const EFI_GUID *a, const EFI_GUID *b)
{
  if (a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3)
  {
    return FALSE;
  }
  for (int i = 0; i < 8; i++)
  {
    if (a->Data4[i] != b->Data4[i])
    {
      return FALSE;
    }
  }
  return TRUE;
}

static BOOLEAN is_ram_type(UINT32 type)
{
  switch (type)
  {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
  case EfiConventionalMemory:
  case EfiACPIReclaimMemory:
  case EfiACPIMemoryNVS:
  case EfiPalCode:
  case EfiPersistentMemory:
    return TRUE;
  default:
    return FALSE;
  }
}

/*
 * Query the firmware memory map and populate BootInfo.
 *
 * Calling AllocatePool() can fragment existing free memory blocks and increase
 * the descriptor count. We allocate descriptor padding and loop until
 * GetMemoryMap() completes without EFI_BUFFER_TOO_SMALL.
 */
static EFI_STATUS init_memory_map(BootInfo *boot_info)
{
  UINTN map_size = 0;
  UINTN map_key = 0;
  UINTN descriptor_size = 0;
  UINT32 descriptor_version = 0;
  EFI_MEMORY_DESCRIPTOR *raw_map = NULL;
  EFI_STATUS status;

  status = g_bs->GetMemoryMap(&map_size, NULL, &map_key, &descriptor_size, &descriptor_version);
  if (status != EFI_BUFFER_TOO_SMALL)
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"GetMemoryMap probe returned unexpected status: ");
    print_hex(status);
    print(u"\r\n");
    return status;
  }

  UINTN allocated_entries = 0;
  while (1)
  {
    map_size += 16 * descriptor_size;
    allocated_entries = map_size / descriptor_size;

    status = g_bs->AllocatePool(EfiLoaderData, map_size, (VOID **)&raw_map);
    if (EFI_ERROR(status))
    {
      print_status_tag(u"FAIL", EFI_LIGHTRED);
      print(u"AllocatePool for raw memory map failed\r\n");
      return status;
    }

    status = g_bs->AllocatePool(EfiLoaderData, allocated_entries * sizeof(MemoryMapEntry), (VOID **)&boot_info->memory_map.entries);
    if (EFI_ERROR(status))
    {
      print_status_tag(u"FAIL", EFI_LIGHTRED);
      print(u"AllocatePool for BootInfo entries failed\r\n");
      g_bs->FreePool(raw_map);
      return status;
    }

    status = g_bs->GetMemoryMap(&map_size, raw_map, &map_key, &descriptor_size, &descriptor_version);
    if (status == EFI_BUFFER_TOO_SMALL)
    {
      g_bs->FreePool(boot_info->memory_map.entries);
      g_bs->FreePool(raw_map);
      boot_info->memory_map.entries = NULL;
      raw_map = NULL;
      continue;
    }
    break;
  }

  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"GetMemoryMap failed: ");
    print_hex(status);
    print(u"\r\n");
    if (raw_map)
    {
      g_bs->FreePool(raw_map);
    }
    return status;
  }

  UINT64 entry_count = map_size / descriptor_size;
  UINT64 total_ram_bytes = 0;
  UINT64 free_ram_bytes = 0;

  boot_info->memory_map.entry_count = entry_count;
  boot_info->memory_map.descriptor_size = descriptor_size;
  boot_info->memory_map.descriptor_version = descriptor_version;
  boot_info->memory_map.map_key = map_key;

  UINT8 *ptr = (UINT8 *)raw_map;
  for (UINT64 i = 0; i < entry_count; i++)
  {
    EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)(ptr + (i * descriptor_size));
    UINT64 region_bytes = desc->NumberOfPages * EFI_PAGE_SIZE;

    boot_info->memory_map.entries[i].address = desc->PhysicalStart;
    boot_info->memory_map.entries[i].size = region_bytes;
    boot_info->memory_map.entries[i].type = desc->Type;
    boot_info->memory_map.entries[i].reserved = 0;

    if (is_ram_type(desc->Type))
    {
      total_ram_bytes += region_bytes;
    }
    if (desc->Type == EfiConventionalMemory)
    {
      free_ram_bytes += region_bytes;
    }
  }

  boot_info->memory_map.total_memory = total_ram_bytes;
  boot_info->memory_map.free_memory = free_ram_bytes;

  g_bs->FreePool(raw_map);

  print_status_tag(u" OK ", EFI_LIGHTGREEN);
  print(u"Memory Map: ");
  print_dec(entry_count);
  print(u" descriptors | Total RAM: ");
  print_dec(total_ram_bytes / (1024 * 1024));
  print(u" MiB | Free RAM: ");
  print_dec(free_ram_bytes / (1024 * 1024));
  print(u" MiB\r\n");

  return EFI_SUCCESS;
}

/*
 * Search the UEFI configuration table array for ACPI RSDP.
 * Matches ACPI 2.0+ GUID first; falls back to ACPI 1.0b if absent.
 */
static EFI_STATUS init_acpi(BootInfo *boot_info)
{
  boot_info->rsdp = 0;

  for (UINTN i = 0; i < g_st->NumberOfTableEntries; i++)
  {
    EFI_CONFIGURATION_TABLE *table = &g_st->ConfigurationTable[i];
    if (guid_equal(&table->VendorGuid, &g_acpi20_guid))
    {
      boot_info->rsdp = (UINT64)table->VendorTable;
      print_status_tag(u" OK ", EFI_LIGHTGREEN);
      print(u"Found ACPI 2.0+ RSDP at: ");
      print_hex(boot_info->rsdp);
      print(u"\r\n");
      return EFI_SUCCESS;
    }
  }

  for (UINTN i = 0; i < g_st->NumberOfTableEntries; i++)
  {
    EFI_CONFIGURATION_TABLE *table = &g_st->ConfigurationTable[i];
    if (guid_equal(&table->VendorGuid, &g_acpi10_guid))
    {
      boot_info->rsdp = (UINT64)table->VendorTable;
      print_status_tag(u" OK ", EFI_YELLOW);
      print(u"Found ACPI 1.0b RSDP at: ");
      print_hex(boot_info->rsdp);
      print(u"\r\n");
      return EFI_SUCCESS;
    }
  }

  print_status_tag(u"WARN", EFI_YELLOW);
  print(u"No ACPI RSDP found in UEFI Configuration Tables\r\n");
  return EFI_NOT_FOUND;
}

/*
 * Acquire linear framebuffer details via Graphics Output Protocol (GOP).
 * Extracts base address, display resolution, scanline pitch, and color masks.
 */
static EFI_STATUS init_framebuffer(BootInfo *boot_info)
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
  EFI_STATUS status = g_bs->LocateProtocol((EFI_GUID *)&g_graphics_output_guid, NULL, (VOID **)&gop);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"LocateProtocol for GOP failed: ");
    print_hex(status);
    print(u"\r\n");
    return status;
  }

  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = gop->Mode->Info;

  boot_info->framebuffer.base_address = (UINT64)gop->Mode->FrameBufferBase;
  boot_info->framebuffer.width = info->HorizontalResolution;
  boot_info->framebuffer.height = info->VerticalResolution;
  boot_info->framebuffer.pixels_per_scanline = info->PixelsPerScanLine;
  boot_info->framebuffer.bytes_per_pixel = 4;

  if (info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)
  {
    boot_info->framebuffer.red_mask = 0x000000FF;
    boot_info->framebuffer.green_mask = 0x0000FF00;
    boot_info->framebuffer.blue_mask = 0x00FF0000;
    boot_info->framebuffer.reserved_mask = 0xFF000000;
  }
  else if (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
  {
    boot_info->framebuffer.blue_mask = 0x000000FF;
    boot_info->framebuffer.green_mask = 0x0000FF00;
    boot_info->framebuffer.red_mask = 0x00FF0000;
    boot_info->framebuffer.reserved_mask = 0xFF000000;
  }
  else if (info->PixelFormat == PixelBitMask)
  {
    boot_info->framebuffer.red_mask = info->PixelInformation.RedMask;
    boot_info->framebuffer.green_mask = info->PixelInformation.GreenMask;
    boot_info->framebuffer.blue_mask = info->PixelInformation.BlueMask;
    boot_info->framebuffer.reserved_mask = info->PixelInformation.ReservedMask;
  }
  else
  {
    print_status_tag(u"WARN", EFI_YELLOW);
    print(u"Non-standard pixel format: ");
    print_dec(info->PixelFormat);
    print(u"\r\n");
  }

  print_status_tag(u" OK ", EFI_LIGHTGREEN);
  print(u"GOP Framebuffer: ");
  print_dec(boot_info->framebuffer.width);
  print(u"x");
  print_dec(boot_info->framebuffer.height);
  print(u" (Pitch: ");
  print_dec(boot_info->framebuffer.pixels_per_scanline);
  print(u") @ ");
  print_hex(boot_info->framebuffer.base_address);
  print(u"\r\n");

  return EFI_SUCCESS;
}

/*
 * Load and parse the freestanding 64-bit ELF kernel from the boot volume.
 *
 * Allocates page-aligned physical memory for PT_LOAD segments, zeroes BSS
 * memory (p_memsz > p_filesz), and extracts the executable entry point.
 */
static EFI_STATUS load_kernel(EFI_HANDLE image_handle, BootInfo *boot_info, const CHAR16 *kernel_path, UINT64 *entry_point)
{
  EFI_LOADED_IMAGE_PROTOCOL *loaded_image = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simple_fs = NULL;
  EFI_FILE_PROTOCOL *root_dir = NULL;
  EFI_FILE_PROTOCOL *kernel_file = NULL;

  EFI_STATUS status = g_bs->HandleProtocol(image_handle, (EFI_GUID *)&g_loaded_image_guid, (VOID **)&loaded_image);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"HandleProtocol(LoadedImage) failed\r\n");
    return status;
  }

  status = g_bs->HandleProtocol(loaded_image->DeviceHandle, (EFI_GUID *)&g_simple_file_system_guid, (VOID **)&simple_fs);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"No filesystem on boot device\r\n");
    return status;
  }

  status = simple_fs->OpenVolume(simple_fs, &root_dir);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"Failed to open root filesystem volume\r\n");
    return status;
  }

  status = root_dir->Open(root_dir, &kernel_file, (CHAR16 *)kernel_path, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"WARN", EFI_YELLOW);
    print(u"Kernel file '");
    print(kernel_path);
    print(u"' not found on ESP\r\n");
    root_dir->Close(root_dir);
    return status;
  }

  UINT64 file_size = 0;
  kernel_file->SetPosition(kernel_file, 0xFFFFFFFFFFFFFFFFULL);
  kernel_file->GetPosition(kernel_file, &file_size);
  kernel_file->SetPosition(kernel_file, 0);

  print_status_tag(u" OK ", EFI_LIGHTGREEN);
  print(u"Found '");
  print(kernel_path);
  print(u"': ");
  print_dec(file_size);
  print(u" bytes\r\n");

  UINT8 *elf_buffer = NULL;
  UINTN read_size = (UINTN)file_size;
  status = g_bs->AllocatePool(EfiLoaderData, read_size, (VOID **)&elf_buffer);
  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"AllocatePool for kernel file failed\r\n");
    kernel_file->Close(kernel_file);
    root_dir->Close(root_dir);
    return status;
  }

  status = kernel_file->Read(kernel_file, &read_size, elf_buffer);
  kernel_file->Close(kernel_file);
  root_dir->Close(root_dir);

  if (EFI_ERROR(status))
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"Failed to read kernel file\r\n");
    g_bs->FreePool(elf_buffer);
    return status;
  }

  Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_buffer;
  UINT32 *magic = (UINT32 *)ehdr->e_ident;
  if (*magic != ELF_MAGIC || ehdr->e_ident[4] != 2 /* 64-bit */ || ehdr->e_machine != 0x3E /* x86_64 */)
  {
    print_status_tag(u"FAIL", EFI_LIGHTRED);
    print(u"Invalid ELF64 x86_64 binary header\r\n");
    g_bs->FreePool(elf_buffer);
    return EFI_LOAD_ERROR;
  }

  Elf64_Phdr *phdrs = (Elf64_Phdr *)(elf_buffer + ehdr->e_phoff);
  UINT64 lowest_addr = 0xFFFFFFFFFFFFFFFFULL;
  UINT64 highest_addr = 0;

  for (UINT16 i = 0; i < ehdr->e_phnum; i++)
  {
    Elf64_Phdr *p = &phdrs[i];
    if (p->p_type == PT_LOAD)
    {
      UINTN pages = EFI_SIZE_TO_PAGES(p->p_memsz);
      EFI_PHYSICAL_ADDRESS dest_addr = p->p_paddr;

      status = g_bs->AllocatePages(AllocateAddress, EfiLoaderData, pages, &dest_addr);
      if (EFI_ERROR(status))
      {
        print_status_tag(u"WARN", EFI_YELLOW);
        print(u"AllocateAddress failed at ");
        print_hex(p->p_paddr);
        print(u", falling back to AnyPages\r\n");

        status = g_bs->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &dest_addr);
        if (EFI_ERROR(status))
        {
          print_status_tag(u"FAIL", EFI_LIGHTRED);
          print(u"AllocatePages failed completely\r\n");
          g_bs->FreePool(elf_buffer);
          return status;
        }
      }

      /* Zero full memsz range to clean BSS, then copy file payload */
      g_bs->SetMem((VOID *)dest_addr, pages * EFI_PAGE_SIZE, 0);

      if (p->p_filesz > 0)
      {
        g_bs->CopyMem((VOID *)dest_addr, elf_buffer + p->p_offset, (UINTN)p->p_filesz);
      }

      if (dest_addr < lowest_addr)
        lowest_addr = dest_addr;
      if (dest_addr + p->p_memsz > highest_addr)
        highest_addr = dest_addr + p->p_memsz;
    }
  }

  *entry_point = ehdr->e_entry;
  boot_info->kernel_physical_base = lowest_addr;
  boot_info->kernel_size = highest_addr - lowest_addr;

  print_status_tag(u" OK ", EFI_LIGHTGREEN);
  print(u"Kernel ELF mapped: ");
  print_hex(lowest_addr);
  print(u" - ");
  print_hex(highest_addr);
  print(u" | Entry: ");
  print_hex(*entry_point);
  print(u"\r\n");

  g_bs->FreePool(elf_buffer);
  return EFI_SUCCESS;
}

/*
 * Standard UEFI entry point.
 */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
  g_st = system_table;
  g_bs = system_table->BootServices;

  /* Cancel firmware watchdog to prevent automatic reboot during kernel load */
  g_bs->SetWatchdogTimer(0, 0, 0, NULL);

  g_st->ConOut->ClearScreen(g_st->ConOut);
  set_color(EFI_LIGHTCYAN, EFI_BACKGROUND_BLACK);
  print(u"  ========================================================\r\n");
  print(u"           UEFI Bootloader Initialization & Probe         \r\n");
  print(u"  ========================================================\r\n\r\n");
  set_color(EFI_LIGHTGRAY, EFI_BACKGROUND_BLACK);

  g_bs->SetMem(&g_boot_info, sizeof(BootInfo), 0);

  init_memory_map(&g_boot_info);
  init_acpi(&g_boot_info);
  init_framebuffer(&g_boot_info);

  UINT64 kernel_entry = 0;
  EFI_STATUS kernel_status = load_kernel(image_handle, &g_boot_info, u"\\kernel.elf", &kernel_entry);

  print(u"\r\n");
  set_color(EFI_YELLOW, EFI_BACKGROUND_BLACK);
  print(u"  --- BootInfo Summary ---\r\n");
  set_color(EFI_LIGHTGRAY, EFI_BACKGROUND_BLACK);

  print(u"    Framebuffer Base : ");
  print_hex(g_boot_info.framebuffer.base_address);
  print(u" (");
  print_dec(g_boot_info.framebuffer.width);
  print(u"x");
  print_dec(g_boot_info.framebuffer.height);
  print(u")\r\n");

  print(u"    ACPI RSDP Table  : ");
  print_hex(g_boot_info.rsdp);
  print(u"\r\n");

  print(u"    Total System RAM : ");
  print_dec(g_boot_info.memory_map.total_memory / (1024 * 1024));
  print(u" MiB (Free: ");
  print_dec(g_boot_info.memory_map.free_memory / (1024 * 1024));
  print(u" MiB)\r\n");

  if (!EFI_ERROR(kernel_status))
  {
    print(u"    Kernel Entry     : ");
    print_hex(kernel_entry);
    print(u" (Ready to Launch)\r\n");

    print(u"\r\n  Press any key to ExitBootServices and launch MyOS Kernel...\r\n");
    UINTN event_index = 0;
    EFI_INPUT_KEY key;
    g_bs->WaitForEvent(1, &g_st->ConIn->WaitForKey, &event_index);
    g_st->ConIn->ReadKeyStroke(g_st->ConIn, &key);

    /*
     * Terminate boot services. Pre-allocate buffer so no memory allocation
     * happens between GetMemoryMap() and ExitBootServices().
     */
    UINTN map_size = 65536;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_ver = 0;
    EFI_MEMORY_DESCRIPTOR *final_map = NULL;

    EFI_STATUS alloc_status = g_bs->AllocatePool(EfiLoaderData, map_size, (VOID **)&final_map);
    if (!EFI_ERROR(alloc_status))
    {
      EFI_STATUS map_status = g_bs->GetMemoryMap(&map_size, final_map, &map_key, &desc_size, &desc_ver);
      if (!EFI_ERROR(map_status))
      {
        EFI_STATUS exit_status = g_bs->ExitBootServices(image_handle, map_key);
        if (EFI_ERROR(exit_status))
        {
          /* Re-fetch map_key and retry if interrupted by firmware timer tick */
          map_status = g_bs->GetMemoryMap(&map_size, final_map, &map_key, &desc_size, &desc_ver);
          if (!EFI_ERROR(map_status))
          {
            g_bs->ExitBootServices(image_handle, map_key);
          }
        }
      }
    }

    /*
     * Transfer control to kernel_main.
     * Pass &g_boot_info in RDI (System V AMD64 ABI) and RCX (Microsoft x64 ABI)
     * to guarantee parameter receipt across compiler configurations.
     */
    __asm__ volatile (
        "movq %0, %%rdi\n\t"
        "movq %0, %%rcx\n\t"
        "call *%1\n\t"
        :
        : "r"(&g_boot_info), "r"(kernel_entry)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );

    /* Halt loop in case kernel returns */
    while (1)
    {
      __asm__ volatile("hlt");
    }
  }
  else
  {
    print(u"\r\n  Kernel not loaded. Press any key to return to UEFI firmware...\r\n");
    UINTN event_index = 0;
    EFI_INPUT_KEY key;
    g_bs->WaitForEvent(1, &g_st->ConIn->WaitForKey, &event_index);
    g_st->ConIn->ReadKeyStroke(g_st->ConIn, &key);
  }

  return EFI_SUCCESS;
}
