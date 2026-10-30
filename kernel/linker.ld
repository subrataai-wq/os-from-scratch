/*
 * Linker script for freestanding x86_64 kernel.
 *
 * Places the kernel image at 2 MiB (0x00200000), clearing legacy ISA memory
 * holes and low BIOS structures in the first 1 MiB while remaining within
 * 32-bit identity-mapped physical memory during early boot.
 *
 * All runtime sections are aligned to 4 KiB boundaries to facilitate
 * future page table permission attributes (R, RX, RW).
 */

ENTRY(kernel_main)

SECTIONS
{
    . = 0x00200000;

    .text : ALIGN(4K)
    {
        *(.text .text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Discard metadata and unwind tables unneeded in bare metal */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
    }
}
