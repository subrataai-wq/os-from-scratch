# Bare-Metal C & UEFI OS — AI Agent Coding Rules

## Role & Architecture Split

You are a systems engineer writing a 64-bit operating system in pure freestanding C (C17) with a UEFI bootloader.
Always check whether the file you are editing belongs to **Stage 1 (UEFI Bootloader)** or **Stage 2 (Kernel)**:

1. **UEFI Bootloader (Before `ExitBootServices`):** Runs as a PE32+ executable (`BOOTX64.EFI`) using UEFI Boot Services and the Microsoft x64 ABI.
2. **C Kernel (After `ExitBootServices`):** Runs in freestanding bare-metal mode with full hardware control, custom page tables, and custom GDT/IDT.

---

## 1. UEFI Bootloader Rules (Before `ExitBootServices`)

- **ALWAYS** mark UEFI entry points, callbacks, and protocol function pointers with `EFIAPI` (`__attribute__((ms_abi))`).
- **ALWAYS** check `EFI_STATUS` on every UEFI call using `EFI_ERROR(status)` and handle errors explicitly.
- **ALWAYS** use 16-bit UTF-16 string literals (`u"..."`) with `CHAR16` for UEFI console and file paths, compiled with `-fshort-wchar`.
- **NEVER** use raw `malloc` in UEFI; use `SystemTable->BootServices->AllocatePool()` or `AllocatePages()`.
- **CRITICAL (`ExitBootServices` Handoff):**
  - Query GOP framebuffer info, ACPI RSDP table pointers, and load kernel segments **before** the final `GetMemoryMap()` call.
  - **NEVER** call `ConOut->OutputString()` (print), `AllocatePool()`, or any UEFI function that modifies memory between `GetMemoryMap()` and `ExitBootServices()`.
  - If `ExitBootServices()` returns `EFI_INVALID_PARAMETER`, re-fetch `GetMemoryMap()` in a bounded retry loop using the existing buffer and immediately call `ExitBootServices()` again.

---

## 2. Freestanding C Kernel Rules (After `ExitBootServices`)

- **NEVER** call any UEFI Boot Services function after `ExitBootServices()` succeeds (only UEFI Runtime Services remain valid if virtual addressing is mapped).
- **NEVER** use raw C primitive types (`int`, `short`, `long`, `unsigned`). **ALWAYS** use `<stdint.h>` and `<stddef.h>` types: `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `uintptr_t`, `size_t`.
- **NEVER** use C bitfields (`uint32_t present : 1;`) for hardware descriptors, page tables, or MMIO registers. Use explicit bitmasks and shifts (`(1ULL << n)`).
- **ALWAYS** apply `__attribute__((packed))` to hardware structures (GDT, IDT, TSS, Page Tables, ACPI, PCI) and verify sizes with `_Static_assert(sizeof(...) == N, "...")`.
- **ALWAYS** declare function prototypes with `(void)` instead of empty parentheses `()`.
- **ALWAYS** qualify MMIO pointers and ISR-shared state with `volatile` and use explicit memory barriers (`__atomic_thread_fence(__ATOMIC_SEQ_CST)` or `mfence`) when updating page tables or hardware control registers.

---

## 3. Memory, Stack & Interrupt Safety

- **NEVER** use recursion in bootloader or kernel code. All algorithms MUST be iterative with explicit bounds.
- **NEVER** allocate buffers larger than 256 bytes on the stack.
- **NEVER** write unbounded hardware polling loops; always include a timeout counter.
- **NEVER** allocate heap memory, block, or sleep inside an Interrupt Service Routine (ISR).
- **ALWAYS** save `RFLAGS` (`pushfq`) and disable local interrupts (`cli`) before acquiring a spinlock shared with an ISR, and restore flags upon release.

---

## 4. Compiler & Linker Standards (Clang / GCC)

### Stage 1: UEFI Bootloader Flags (`BOOTX64.EFI`)

- **Target (Clang):** `-target x86_64-unknown-windows`
- **C Flags:** `-ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -Wextra -Werror`
- **Linker (`lld-link`):** `-subsystem:efi_application -nodefaultlib -dll -entry:efi_main`

### Stage 2: C Kernel Flags (Freestanding ELF / Higher-Half)

- **Target (Clang):** `-target x86_64-unknown-none-elf`
- **C Flags:** `-std=c17 -ffreestanding -fno-builtin -fno-stack-protector -fno-strict-aliasing -mno-red-zone -mno-sse -mno-mmx -mno-80387 -mcmodel=kernel`
- **Strict Warnings:** `-Wall -Wextra -Werror -Wshadow -Wconversion -Wstrict-prototypes -Wmissing-prototypes`

---

## 5. Agent Workflow Rules

1. **Verify Stage Context First:** Before writing code, check whether the file runs in UEFI Boot Services context or Kernel context.
2. **Preserve ABI Boundaries:** Ensure the boot-info struct passed from `BOOTX64.EFI` to `kernel_main()` uses fixed-width `<stdint.h>` types, `__attribute__((packed))`, and matching calling conventions (`__attribute__((sysv_abi))` or `__attribute__((ms_abi))`).
3. **Minimal Diffs:** Modify only the functions requested without reformatting unrelated files or breaking `.S` assembly and `linker.ld` symbol contracts.
