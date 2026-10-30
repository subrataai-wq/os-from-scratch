#include "efi.h"

static EFI_SYSTEM_TABLE *ST;
static EFI_BOOT_SERVICES *BS;

static void print(CHAR16 *s) { ST->ConOut->OutputString(ST->ConOut, s); }

static void print_hex(UINT64 v)
{
  CHAR16 b[19];
  b[0] = u'0';
  b[1] = u'x';
  for (int i = 0; i < 16; i++)
    b[2 + i] = u"0123456789ABCDEF"[(v >> (60 - 4 * i)) & 0xF];
  b[18] = 0;
  print(b);
}
static void print_dec(UINT64 v)
{
  CHAR16 b[21];
  int i = 20;
  b[i] = 0;
  do
  {
    b[--i] = u'0' + (CHAR16)(v % 10);
    v /= 10;
  } while (v);
  print(&b[i]);
}
static void fail(CHAR16 *what, EFI_STATUS s)
{
  print(what);
  print(u" failed: ");
  print_hex(s);
  print(u"\r\n");
}

static int guid_eq(const EFI_GUID *a, const EFI_GUID *b)
{
  if (a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3)
    return 0;
  for (int i = 0; i < 8; i++)
    if (a->Data4[i] != b->Data4[i])
      return 0;
  return 1;
}

/* 1. Memory map (§7.2.3) */
static void step_memmap(void)
{
  UINTN size = 0, key, dsize;
  UINT32 dver;
  EFI_MEMORY_DESCRIPTOR *map = NULL;

  EFI_STATUS s = BS->GetMemoryMap(&size, NULL, &key, &dsize, &dver); /* expect BUFFER_TOO_SMALL */
  if (s != EFI_BUFFER_TOO_SMALL)
  {
    fail(u"GetMemoryMap(size)", s);
    return;
  }
  size += 2 * dsize; /* AllocatePool itself adds map entries */
  s = BS->AllocatePool(EfiLoaderData, size, (VOID **)&map);
  if (EFI_ERROR(s))
  {
    fail(u"AllocatePool", s);
    return;
  }
  s = BS->GetMemoryMap(&size, map, &key, &dsize, &dver);
  if (EFI_ERROR(s))
  {
    fail(u"GetMemoryMap", s);
    return;
  }

  UINT64 free_pages = 0, count = 0;
  for (UINT8 *p = (UINT8 *)map; p < (UINT8 *)map + size; p += dsize)
  { /* step by dsize, NOT sizeof */
    EFI_MEMORY_DESCRIPTOR *d = (EFI_MEMORY_DESCRIPTOR *)p;
    count++;
    if (d->Type == EfiConventionalMemory)
      free_pages += d->NumberOfPages;
  }
  print(u"Memory map: ");
  print_dec(count);
  print(u" regions, ");
  print_dec(free_pages * 4096 / (1024 * 1024));
  print(u" MiB free\r\n");
  BS->FreePool(map);
}

/* 2. Configuration tables (§4.6) - find the ACPI RSDP */
static void step_acpi(void)
{
  for (UINTN i = 0; i < ST->NumberOfTableEntries; i++)
  {
    if (guid_eq(&ST->ConfigurationTable[i].VendorGuid, &gEfiAcpi20Guid))
    {
      print(u"ACPI 2.0 RSDP at ");
      print_hex((UINT64)ST->ConfigurationTable[i].VendorTable);
      print(u"\r\n");
      return;
    }
  }
  print(u"No ACPI 2.0 table\r\n");
}

/* 3. Read a file from the volume we booted from (§9.1, §13.4, §13.5) */
static void step_file(EFI_HANDLE image, CHAR16 *path)
{
  EFI_LOADED_IMAGE_PROTOCOL *li;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
  EFI_FILE_PROTOCOL *root, *f;
  EFI_STATUS s;

  s = BS->HandleProtocol(image, &gEfiLoadedImageGuid, (VOID **)&li);
  if (EFI_ERROR(s))
  {
    fail(u"LoadedImage", s);
    return;
  }
  s = BS->HandleProtocol(li->DeviceHandle, &gEfiSimpleFileSystemGuid, (VOID **)&fs);
  if (EFI_ERROR(s))
  {
    fail(u"SimpleFileSystem", s);
    return;
  }
  s = fs->OpenVolume(fs, &root);
  if (EFI_ERROR(s))
  {
    fail(u"OpenVolume", s);
    return;
  }
  s = root->Open(root, &f, path, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(s))
  {
    fail(u"Open", s);
    root->Close(root);
    return;
  }

  UINT64 size; /* seek to end to learn the size */
  f->SetPosition(f, 0xFFFFFFFFFFFFFFFFULL);
  f->GetPosition(f, &size);
  f->SetPosition(f, 0);

  UINT8 *buf;
  UINTN n = (UINTN)size;
  s = BS->AllocatePool(EfiLoaderData, n + 1, (VOID **)&buf);
  if (!EFI_ERROR(s) && !EFI_ERROR(f->Read(f, &n, buf)))
  {
    print(u"Read ");
    print_dec(n);
    print(u" bytes, first byte ");
    print_hex(buf[0]);
    print(u"\r\n");
  }
  else
    fail(u"Read", s);
  f->Close(f);
  root->Close(root);
}

/* 4. Graphics: linear framebuffer via GOP (§12.9) */
static void step_gop(void)
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  EFI_STATUS s = BS->LocateProtocol(&gEfiGraphicsOutputGuid, NULL, (VOID **)&gop);
  if (EFI_ERROR(s))
  {
    fail(u"GOP", s);
    return;
  }
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *i = gop->Mode->Info;
  if (i->PixelFormat > 1)
  {
    print(u"Unsupported pixel format\r\n");
    return;
  }

  UINT32 *fb = (UINT32 *)gop->Mode->FrameBufferBase;
  for (UINT32 y = 0; y < i->VerticalResolution; y++)
    for (UINT32 x = 0; x < i->HorizontalResolution; x++)
    {
      UINT32 r = x * 255 / i->HorizontalResolution, g = y * 255 / i->VerticalResolution, b = 128;
      fb[y * i->PixelsPerScanLine + x] = /* use the scanline pitch, not the width */
          (i->PixelFormat == 1) ? (r << 16) | (g << 8) | b : (b << 16) | (g << 8) | r;
    }
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *st)
{
  ST = st;
  BS = st->BootServices;
  BS->SetWatchdogTimer(0, 0, 0, NULL); /* see §7.5: firmware arms a watchdog for launched images */
  ST->ConOut->ClearScreen(ST->ConOut);
  print(u"Stage 1\r\n");

  step_memmap();
  step_acpi();
  step_file(image, u"\\hello.txt"); /* put any file at esp\hello.txt */
  step_gop();                       /* last: it draws over the text console */

  UINTN idx;
  EFI_INPUT_KEY k;
  BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &idx);
  ST->ConIn->ReadKeyStroke(ST->ConIn, &k);
  return EFI_SUCCESS; /* returning exits the image back to firmware (§4.1) */
}