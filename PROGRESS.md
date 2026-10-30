# Project Progress & Task Tracker 🚀

This document tracks completed tasks, pending roadmap requirements, and engineering logs for this operating system project.

---

## 📍 Where the Project Stands

```text
[X] Stage 1: UEFI Application   ← CURRENT STATUS (Completed / In Verification)
[ ] Stage 2: Handoff            (Next: Load kernel ELF, ExitBootServices, Jump)
[ ] Stage 3: Kernel Phase       (Future: GDT, IDT, Paging, Drivers, Scheduler)
```

---

## 📋 Task Checklist (Stage 1)

### ✅ Completed

- **Core Runtime:** Entry point setup (`efi_main`), `EFI_SYSTEM_TABLE` mapping, and Watchdog disabled.
- **Console I/O:** Text printing (`OutputString`), `ClearScreen`, and keyboard input (`WaitForKey`, `ReadKeyStroke`).
- **Memory Management:** Pool allocation (`AllocatePool`) and memory map parsing (100 regions, 206 MiB free).
- **Protocols & Files:** GUID resolution (`HandleProtocol`/`LocateProtocol`), `LoadedImage` boot disk location, and file reads (`hello.txt`, 35 bytes).
- **Hardware Discovery:** ACPI RSDP located in firmware configuration tables.
- **Toolchain:** Working CMake presets, Clang compiler, LLD-Linker, and QEMU + EDK2 execution environment.

### 🔍 Needs Visual/Manual Confirmation

- [x] Verify the GOP gradient displays properly in QEMU (Red left-to-right, Green top-to-bottom).
- [x] Confirm the application waits for a keystroke before exiting cleanly back to the firmware interface.
- [x] Run `llvm-objdump -p build\BOOTX64.EFI | findstr /i "Magic Subsystem"` to verify magic `020b` and subsystem `10`.

### ⚠️ Blocks Stage 2 (Must Do Next)

- [ ] **Print GOP Details:** Output resolution, pixel format, `FrameBufferBase`, `FrameBufferSize`, and `PixelsPerScanLine` (required to pass display specs to the kernel).
- [ ] **Implement `AllocatePages`:** Switch from pool allocation to page-aligned memory allocation (required to place the kernel at a fixed address).

### 💡 Suggested Optimizations & Extras

- [ ] Print a categorized breakdown of the memory map by type (Free, Loader, Firmware, ACPI, Reserved).
- [ ] Output the explicit image base address and size fetched from `LoadedImage`.
- [ ] Find the SMBIOS table (`SMBIOS3_TABLE_GUID`).
- [ ] Test raw disk access via Block I/O.

---

## 🛠️ Architecture & Toolchain Insights

### 1. Core Implementation Details

- **`efi.h`**: A custom standalone header typed manually from the official UEFI specifications. It maps system tables, boot services, console protocols, files, graphics, and memory structures with precise field ordering.
- **`main.c`**: The main entry point (`efi_main`). Upon receiving the system table pointer from firmware, it executes text output, memory map parsing, ACPI discovery, file reading, and direct pixel framebuffer writes.

### 2. Environment Configurations

The build system handles host OS paths and enforces the cross-compilation pipeline using the following setup:

| Tool                   | Role                                                                   |
| :--------------------- | :--------------------------------------------------------------------- |
| **Clang**              | Compiles C source code with explicit cross-compilation target flags.   |
| **LLD-Link**           | Links object files into a PE32+ `.EFI` binary format required by UEFI. |
| **CMake + Ninja**      | Coordinates consistent, repeatable builds across hosts.                |
| **QEMU + EDK2 (OVMF)** | Emulates a virtual PC with real UEFI firmware for rapid testing.       |

### 3. Debugging Log & Workarounds

- **Git Bash Path Resolution:** Git Bash dynamically rewrote the linker flag `/nodefaultlib` into a broken directory path.
  - _Fix:_ Switched to dash-style linker flags and executed builds from PowerShell.
- **MSYS2 Environment Targeting:** MSYS2's default clang target assumed a MinGW environment, causing it to reject `-fuse-ld=lld-link`.
  - _Fix:_ Enforced explicit cross-compilation via `--target=x86_64-unknown-windows`.

---

## ⚡ Verified Build Commands

```bash
# Configure the build environment
cmake --preset uefi

# Compile the project
cmake --build --preset uefi

# Launch the operating system image inside QEMU
cmake --build --preset uefi --target run
```

---

## 📊 Sample Boot Output Log

```text
Stage 1
Memory map: 100 regions, 206 MiB free
ACPI 2.0 RSDP at 0x000000000F77E014
Read 35 bytes, first byte 0x0000000000000048
```
