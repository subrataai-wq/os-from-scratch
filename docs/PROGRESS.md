# Implementation Status & Roadmap

Tracks milestone completion, runtime specifications, and pending kernel subsystems.

## Current Status

```text
[x] Stage 1: UEFI Bootloader & Hardware Probing
[x] Stage 2: ELF64 Loader & Kernel Handoff
[ ] Stage 3: Core Kernel Subsystems (In Progress)
```

## Milestone Checklist

### Stage 1: UEFI Environment & Probing (Complete)

- [x] Standalone UEFI 2.11 headers (`include/uefi/`) without external SDK dependencies.
- [x] Memory map acquisition and available RAM calculation (`GetMemoryMap`).
- [x] ACPI 2.0+ RSDP detection via system configuration table (with ACPI 1.0b fallback).
- [x] Linear framebuffer discovery via Graphics Output Protocol (GOP).
- [x] Boot media volume access via `SimpleFileSystem` and `LoadedImage`.

### Stage 2: Kernel Loader & Handoff (Complete)

- [x] Defined unified `BootInfo` handoff structure (`boot_info.h`).
- [x] ELF64 header validation (`ELF_MAGIC`, 64-bit, x86_64).
- [x] Page-aligned memory allocation for `PT_LOAD` segments (`AllocatePages`).
- [x] BSS segment zero-initialization (`p_memsz > p_filesz`).
- [x] Clean termination of firmware runtime (`ExitBootServices`) with timer-tick retry loop.
- [x] Dual-ABI register handoff (`BootInfo*` in `rdi` and `rcx`) and jump to `kernel_main`.

### Stage 3: Kernel Core (In Progress)

- [x] Freestanding x86_64 ELF target linked at 2 MiB (`0x00200000`).
- [x] Linear framebuffer driver with dynamic RGB/BGR color conversion and clipping.
- [x] 8x16 VGA bitmap font renderer with automated vertical scrolling.
- [x] Freestanding memory routines (`memcpy`, `memset`, `memmove`, `memcmp`, `strlen`).
- [ ] **Next:** Global Descriptor Table (GDT) and Task State Segment (TSS).
- [ ] **Next:** Interrupt Descriptor Table (IDT) and CPU exception handlers (ISRs).
- [ ] **Next:** Physical Memory Manager (PMM) bitmap allocator using firmware memory map.
- [ ] **Backlog:** 4-level paging and virtual memory manager (VMM).
- [ ] **Backlog:** Local APIC timer and PS/2 keyboard interrupt driver.

## Technical Specifications

### Memory Layout

| Address | Region | Description |
| :--- | :--- | :--- |
| `0x00000000 - 0x000FFFFF` | Low Memory (1 MiB) | Reserved (BIOS/IVT/BDA/VGA legacy holes) |
| `0x00200000` | Kernel Base (2 MiB) | Freestanding ELF image (`.text`, `.rodata`, `.data`, `.bss`) |
| Dynamic | Framebuffer | Physical linear address mapped via GOP |
| Dynamic | System RAM | Described by `BootInfo.memory_map` |

### Kernel Handoff State

- **Execution Mode:** x86_64 Long Mode (64-bit).
- **Interrupts:** Disabled (`IF=0`).
- **Paging:** Identity-mapped by UEFI boot services page tables.
- **Calling Convention:** Dual-ABI compatibility; `BootInfo*` passed in both `RDI` (System V AMD64) and `RCX` (Microsoft x64).

### Key Implementation Notes

- **`ExitBootServices` Invalidation:** Memory map keys are invalidated if memory allocations occur after `GetMemoryMap`. A dedicated 64 KB descriptor buffer is pre-allocated to ensure the final call directly precedes exit.
- **Color Format Normalization:** GOP displays vary between BGRX8888 and RGBX8888. Pixel writing inspects `red_mask` / `blue_mask` at runtime rather than assuming standard PC color ordering.
- **Page-Aligned Loading:** ELF segments are allocated via `AllocatePages(AllocateAddress)` to honor physical load addresses specified by `linker.ld`.
