# Contributing Guidelines

Thank you for contributing to `os-from-scratch`. This project is a minimal, freestanding x86_64 operating system booting natively via modern UEFI.

To maintain code quality, architectural consistency, and binary stability, please adhere to the following standards when proposing changes.

## Development Environment & Toolchain

All code is compiled for bare-metal x86_64 targets using Clang and LLD:
- **UEFI Bootloader (`BOOTX64.EFI`)**: Target `x86_64-unknown-windows`, linked via `lld-link` with `-subsystem:efi_application`.
- **Kernel (`kernel.elf`)**: Target `x86_64-unknown-none-elf`, linked via `ld.lld` with custom `kernel/linker.ld`.

Builds must compile with zero errors and zero warnings:

```powershell
cmake --preset ninja-debug
cmake --build --preset ninja-debug
cmake --build build/debug --target run
```

## Code Standards

1. **Freestanding C Runtime:**
   - Standard C libraries (`libc`) are not available. Do not include `<stdio.h>`, `<stdlib.h>`, or `<string.h>`.
   - Use freestanding headers (`<stdint.h>`, `<stddef.h>`, `<stdbool.h>`) or kernel equivalents (`kernel/string.h`, `kernel/screen.h`).
   - Kernel code runs with `-mno-red-zone` and `-fno-stack-protector`. Ensure all stack buffers and memory operations are safe.

2. **Documentation & Comments:**
   - Document technical rationale, hardware constraints, and memory invariants (the *why*, not the *what*).
   - Avoid trivial narration (e.g. `// loop through array` or `// clear screen`).
   - Avoid decorative ASCII dividers (`/* ================= */`) and tutorial step numbering.
   - Use standard C block comments (`/* ... */`).

3. **Memory & ABI Invariants:**
   - Any modifications to the bootloader-kernel handoff must update [`boot_info.h`](boot_info.h) and preserve 64-bit alignment.
   - Kernel memory accesses must respect the 2 MiB physical load address and page boundaries.

## Git Workflow & Commit Guidelines

- **Branching:** Create a descriptive branch from `main` (e.g. `feat/gdt-setup`, `fix/gop-stride`).
- **Commit Messages:** Follow the Conventional Commits / Linux kernel style:
  - Subject line: imperative mood, lowercase, concise (<= 50 characters).
  - Body: explain the problem being solved, technical decisions, and hardware implications.
- **Verification:** Before submitting, verify that the project builds cleanly from scratch and boots to the idle halt state in QEMU.

## License

This project is licensed under the **GNU General Public License v2 (GPL-2.0)**. By submitting code or documentation to this repository, you agree that your contributions are licensed under the terms of GPLv2 as stated in [LICENSE](LICENSE).

