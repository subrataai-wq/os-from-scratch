**Universal Reference Guide: UEFI Bare-Metal Development Toolchain**

A practical handbook for writing freestanding UEFI applications in C — covering compilation, linking, filesystem layout, firmware, and QEMU emulation.

---

### Table of Contents

1. [Workflow Architecture Overview](#1-workflow-architecture-overview)
2. [The Compiler: Clang](#2-the-compiler-clang)
3. [The Linker: LLD (`lld-link`)](#3-the-linker-lld-link)
4. [The Target Filesystem: EFI System Partition (ESP)](#4-the-target-filesystem-efi-system-partition-esp)
5. [The Firmware: EDK2 & OVMF](#5-the-firmware-edk2--ovmf)
6. [The Emulator: QEMU](#6-the-emulator-qemu)
7. [Demo Project Structure](#7-demo-project-structure)
8. [Toolchain Cheat Sheet](#8-toolchain-cheat-sheet)

---

### 1. Workflow Architecture Overview

Bare-metal UEFI development runs **without an OS and without a standard C runtime (libc)**. The firmware loads and executes your binary directly.

```text
src/main.c
    │  clang -c  (freestanding, short-wchar, no red-zone)
    ▼
build/main.o          (COFF x86-64 object)
    │  lld-link       (-subsystem:efi_application, -nodefaultlib, -entry:efi_main)
    ▼
BOOTX64.EFI           (PE32+ executable, Subsystem 0x000A)
    │  staged into fallback path
    ▼
esp/EFI/BOOT/BOOTX64.EFI
    │  QEMU mounts directory as FAT + loads OVMF flash images
    ▼
qemu-system-x86_64    (firmware + your application)
```

---

### 2. The Compiler: Clang

#### Why Clang?

- Single binary can cross-compile to `x86_64`, `aarch64`, `riscv64`, etc.
- Native support for PE/COFF and the Microsoft x64 calling convention (`ms_abi`) required by UEFI.

#### Recommended Compile Flags

```bash
clang --target=x86_64-unknown-windows \
      -ffreestanding -fshort-wchar \
      -mno-red-zone -fno-stack-protector \
      -c src/main.c -o build/main.o
```

| Flag                              | Purpose                                | Risk if omitted                                                        |
| --------------------------------- | -------------------------------------- | ---------------------------------------------------------------------- |
| `--target=x86_64-unknown-windows` | Produce COFF objects + Windows x64 ABI | Linker rejects ELF vs PE/COFF mismatch                                 |
| `-ffreestanding`                  | No libc, no standard startup code      | Compiler may emit calls to `memcpy`/`memset` or assume a normal `main` |
| `-fshort-wchar`                   | `wchar_t` = 2 bytes (UTF-16)           | Wide strings become 4 bytes → text corruption in UEFI                  |
| `-mno-red-zone`                   | Disable 128-byte red zone below `%rsp` | Interrupt/exception handlers can clobber data below the stack pointer  |
| `-fno-stack-protector`            | No stack canaries                      | Unresolved `__stack_chk_fail` at link time                             |
| `-c`                              | Compile only (no link)                 | Clang tries to link against host libraries and fails                   |

#### Calling Convention (`EFIAPI` / `ms_abi`)

On x86_64, UEFI uses the Microsoft x64 calling convention:

- First 4 integer/pointer arguments: `%rcx`, `%rdx`, `%r8`, `%r9`
- Caller must reserve 32 bytes of shadow space on the stack
- Mark functions with `__attribute__((ms_abi))` (commonly via `#define EFIAPI`)

---

### 3. The Linker: LLD (`lld-link`)

`lld-link` (LLVM’s replacement for Microsoft `link.exe`) turns COFF objects into PE32+ images.

#### UEFI PE32+ Subsystems

| Subsystem                                 | Value         | Meaning                        |
| ----------------------------------------- | ------------- | ------------------------------ |
| `IMAGE_SUBSYSTEM_EFI_APPLICATION`         | `0x000A` (10) | UEFI application (what we use) |
| `IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER` | `0x000B` (11) | Boot-service driver            |
| `IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER`      | `0x000C` (12) | Runtime driver                 |

#### Recommended Link Flags

```bash
lld-link -subsystem:efi_application \
         -entry:efi_main \
         -nodefaultlib \
         build/main.o \
         -out:build/BOOTX64.EFI
```

| Flag                         | Purpose                                      | Risk if omitted                                             |
| ---------------------------- | -------------------------------------------- | ----------------------------------------------------------- |
| `-subsystem:efi_application` | Sets PE Optional Header Subsystem = `0x000A` | Firmware refuses to load the image                          |
| `-entry:efi_main`            | Sets entry point to your symbol              | Linker looks for CRT startup (`mainCRTStartup`) and fails   |
| `-nodefaultlib`              | Do not pull in any default libraries         | Linker tries to resolve host OS libraries that do not exist |

> **Tip:** Prefer dash-style flags (`-nodefaultlib`) over slash-style (`/nodefaultlib`). Some Unix shells on Windows treat leading `/` as an absolute path.

#### Inspecting the Binary

```bash
llvm-objdump -p build/BOOTX64.EFI
```

Expected output (key fields):

```text
Magic                   020b            (PE32+)
AddressOfEntryPoint     0000000000001000
Subsystem               0000000a        (EFI application)
```

---

### 4. The Target Filesystem: EFI System Partition (ESP)

#### Default Boot Paths (UEFI Spec §3.5.1)

If no NVRAM boot entries exist, firmware searches a FAT12/16/32 volume for:

| Architecture | Path                        |
| ------------ | --------------------------- |
| x86-64       | `\EFI\BOOT\BOOTX64.EFI`     |
| AArch64      | `\EFI\BOOT\BOOTAA64.EFI`    |
| x86-32       | `\EFI\BOOT\BOOTIA32.EFI`    |
| RISC-V 64    | `\EFI\BOOT\BOOTRISCV64.EFI` |

#### QEMU Virtual FAT

Instead of creating a real disk image, QEMU can present a host directory as a FAT volume:

```bash
-drive format=raw,file=fat:rw:esp
```

The directory tree under `esp/` appears inside the guest as a FAT filesystem.

---

### 5. The Firmware: EDK2 & OVMF

**OVMF** (Open Virtual Machine Firmware) is the EDK II port that provides full UEFI firmware for QEMU.

#### Split Firmware Images

Modern OVMF ships two separate files:

| File                       | Role                                  | Access     |
| -------------------------- | ------------------------------------- | ---------- |
| `CODE.fd` / `OVMF_CODE.fd` | Firmware code (SEC → PEI → DXE → BDS) | Read-only  |
| `VARS.fd` / `OVMF_VARS.fd` | NVRAM (boot order, variables, etc.)   | Read-write |

#### Correct Flash Mapping

Do **not** use the legacy `-bios` flag for modern OVMF. Use parallel flash devices:

```bash
-drive if=pflash,format=raw,unit=0,file=.../OVMF_CODE.fd,readonly=on
-drive if=pflash,format=raw,unit=1,file=.../OVMF_VARS.fd,snapshot=on
```

- `unit=0` → code (read-only)
- `unit=1` → variables (writable)
- `snapshot=on` → runtime writes go to a temporary overlay; the host file stays clean and permission problems are avoided

---

### 6. The Emulator: QEMU

#### Recommended Command Line

```bash
qemu-system-x86_64 \
  -drive if=pflash,format=raw,unit=0,file="/path/to/OVMF_CODE.fd",readonly=on \
  -drive if=pflash,format=raw,unit=1,file="/path/to/OVMF_VARS.fd",snapshot=on \
  -drive format=raw,file=fat:rw:esp \
  -net none
```

| Flag                | Purpose                              |
| ------------------- | ------------------------------------ |
| `if=pflash`         | Attach to parallel flash controller  |
| `unit=0` / `unit=1` | Code vs. variables                   |
| `readonly=on`       | Protect firmware code                |
| `snapshot=on`       | Keep host VARS file pristine         |
| `fat:rw:esp`        | Present host directory as FAT volume |
| `-net none`         | Skip network/PXE boot delays         |

#### Headless / Serial Console

```bash
qemu-system-x86_64 ... -nographic -serial mon:stdio
```

- `-nographic` — no GUI window
- `-serial mon:stdio` — serial + monitor on the terminal
- Exit: `Ctrl+A` then `X`

#### GDB Debugging

```bash
# Start QEMU frozen
qemu-system-x86_64 ... -s -S

# In another terminal
gdb
(gdb) target remote :1234
(gdb) continue
```

`-s` opens a GDB server on TCP 1234; `-S` stops at the first instruction.

---

### 7. Demo Project Structure

```text
my-project/
├── build/                  # Intermediate objects + final EFI
│   ├── main.o
│   └── BOOTX64.EFI
├── esp/                    # Staging root for QEMU FAT
│   └── EFI/
│       └── BOOT/
│           └── BOOTX64.EFI
├── src/
│   ├── efi.h               # Minimal UEFI types / protocols
│   └── main.c              # Entry point (efi_main)
├── CMakeLists.txt          # Optional build automation
└── TOOLCHAIN.md            # This guide (or your notes)
```

---

### 8. Toolchain Cheat Sheet

| Task                | Command                                                                                                                                                                                  |
| ------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Compile**         | `clang --target=x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector -c src/main.c -o build/main.o`                                                    |
| **Link**            | `lld-link -subsystem:efi_application -entry:efi_main -nodefaultlib build/main.o -out:build/BOOTX64.EFI`                                                                                  |
| **Inspect headers** | `llvm-objdump -p build/BOOTX64.EFI`                                                                                                                                                      |
| **Disassemble**     | `llvm-objdump -d build/main.o`                                                                                                                                                           |
| **Run (graphical)** | `qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,file=CODE.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=VARS.fd,snapshot=on -drive format=raw,file=fat:rw:esp -net none` |
| **Run (headless)**  | Add `-nographic -serial mon:stdio`                                                                                                                                                       |
| **Debug**           | Add `-s -S` then connect with `gdb` → `target remote :1234`                                                                                                                              |
