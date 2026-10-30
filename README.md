# os-from-scratch

A minimal 64-bit operating system for x86_64, booting natively via UEFI without legacy BIOS or GRUB dependencies. Implemented in freestanding C using Clang, LLD, and Ninja.

![Kernel Boot Screen](images/gui-output1.png)

## Overview

The project consists of two primary binaries:

1. **Bootloader (`BOOTX64.EFI`)**: A PE32+ UEFI application that probes system hardware (ACPI 2.0+ RSDP, memory map, GOP framebuffer), parses and loads the 64-bit ELF kernel image, exits boot services, and transfers control via a unified handoff structure (`BootInfo`).
2. **Kernel (`kernel.elf`)**: A freestanding x86_64 ELF binary loaded at 2 MiB physical. Implements direct linear framebuffer rendering (RGB and BGR formats), an 8x16 VGA font engine, and a scrolling text console.

## Project Structure

```text
├── boot_info.h       # Bootloader-to-kernel handoff contract
├── CMakeLists.txt    # Unified build configuration for bootloader and kernel
├── CMakePresets.json # Ninja build presets (debug / release)
├── CONTRIBUTING.md   # Contribution guidelines and coding standards
├── LICENSE           # GNU General Public License v2
├── main.c            # UEFI bootloader, hardware probing, and ELF64 loader
├── include/uefi/     # Standalone UEFI 2.x specification headers
├── kernel/
│   ├── kernel.c      # Kernel entry point (kernel_main)
│   ├── screen.c      # Framebuffer terminal and primitive rendering
│   ├── font.h        # 8x16 bitmap console font
│   ├── string.c      # Freestanding memory routines (memcpy, memset, etc.)
│   └── linker.ld     # Kernel linker script (base at 0x00200000)
└── docs/
    └── PROGRESS.md   # Current milestone status and technical notes
```

## Prerequisites

- **Clang** (with `lld` / `lld-link`)
- **CMake** (>= 3.20) and **Ninja**
- **QEMU** (`qemu-system-x86_64`) with OVMF firmware

## Building and Running

Configure and compile using CMake presets:

```powershell
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

Or using standard CMake:

```powershell
cmake -B build/debug -G Ninja
cmake --build build/debug
```

Launch under QEMU:

```powershell
cmake --build build/debug --target run
```

The build system automatically deploys `BOOTX64.EFI` to `build/debug/esp/EFI/BOOT/` and `kernel.elf` to `build/debug/esp/` for direct FAT volume emulation.

## Status & Roadmap

See [docs/PROGRESS.md](docs/PROGRESS.md) for milestone tracking, handoff invariants, and upcoming kernel tasks (GDT, IDT, PMM, Paging).

## License & Contributing

Licensed under the [GNU General Public License v2](LICENSE). Contributions welcome — please review [CONTRIBUTING.md](CONTRIBUTING.md) before submitting patches or pull requests.
