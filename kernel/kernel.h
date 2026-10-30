/*
 * kernel.h - Freestanding x86_64 kernel core interfaces.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "../boot_info.h"
#include "screen.h"

/*
 * Kernel entry point invoked by the UEFI bootloader after ExitBootServices().
 * Receives the BootInfo structure with identity-mapped physical pointers.
 */
void kernel_main(BootInfo *boot_info);

#endif /* KERNEL_H */
