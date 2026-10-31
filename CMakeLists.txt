cmake_minimum_required(VERSION 3.20)

# Freestanding cross-compilation requires Clang with LLD support
if(NOT DEFINED CMAKE_C_COMPILER)
    find_program(CLANG_C_COMPILER NAMES clang clang.exe)
    if(CLANG_C_COMPILER)
        set(CMAKE_C_COMPILER "${CLANG_C_COMPILER}" CACHE FILEPATH "C compiler")
    endif()
endif()


# Target bare-metal system architecture (x86_64)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
project(myos LANGUAGES C)


if(CMAKE_C_COMPILER_ID AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "This project requires Clang with LLD (detected: ${CMAKE_C_COMPILER_ID}). "
        "Please configure with -D CMAKE_C_COMPILER=clang or use the CMake presets."
    )
endif()

find_program(LLD_LINKER NAMES ld.lld ld.lld.exe lld lld.exe REQUIRED)

set(ESP_DIR "${CMAKE_BINARY_DIR}/esp")

# ------------------------------------------------------------------------------
# Kernel Target (x86_64 Freestanding ELF64)
# ------------------------------------------------------------------------------

set(KERNEL_SOURCES
    kernel/kernel.c
    kernel/screen.c
    kernel/string.c
)

add_library(kernel_objs OBJECT ${KERNEL_SOURCES})

target_include_directories(kernel_objs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
)

target_compile_options(kernel_objs PRIVATE
    --target=x86_64-unknown-none-elf
    -nostdlib
    -ffreestanding
    -fno-builtin
    -fno-strict-aliasing
    -mno-red-zone
    -mgeneral-regs-only
    -fno-stack-protector
    -Wall
    -Wextra
    -Werror
)

set(KERNEL_ELF "${CMAKE_BINARY_DIR}/kernel.elf")
set(KERNEL_LDS "${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ld")

add_custom_command(
    OUTPUT ${KERNEL_ELF}
    COMMAND "${LLD_LINKER}" -m elf_x86_64 -T "${KERNEL_LDS}" -o "${KERNEL_ELF}" $<TARGET_OBJECTS:kernel_objs>
    COMMAND_EXPAND_LISTS
    DEPENDS kernel_objs "${KERNEL_LDS}"
    COMMENT "Linking kernel.elf with ld.lld..."
)

add_custom_target(kernel ALL
    DEPENDS ${KERNEL_ELF}
)

# Stage kernel at the root of the EFI System Partition
add_custom_command(TARGET kernel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ESP_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${KERNEL_ELF}" "${ESP_DIR}/kernel.elf"
    COMMENT "Deploying kernel.elf to EFI System Partition (esp/kernel.elf)..."
)

# Register ESP staging directory for cleanup during 'clean' target
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${ESP_DIR}")

# ------------------------------------------------------------------------------
# UEFI Bootloader Target (PE32+ EFI Application)
# ------------------------------------------------------------------------------

add_executable(bootx64 main.c)

option(OS_AUTOBOOT "Bypass interactive keypress prompt before ExitBootServices" OFF)
if(OS_AUTOBOOT)
    target_compile_definitions(bootx64 PRIVATE OS_AUTOBOOT=1)
endif()

set_target_properties(bootx64 PROPERTIES
    OUTPUT_NAME "BOOTX64"
    SUFFIX ".EFI"
)

target_include_directories(bootx64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(bootx64 PRIVATE
    --target=x86_64-unknown-windows
    -nostdlib
    -ffreestanding
    -fshort-wchar
    -mno-red-zone
    -mms-bitfields
    -fno-stack-protector
    -Wall
    -Wextra
    -Werror
)

target_link_options(bootx64 PRIVATE
    --target=x86_64-unknown-windows
    -nostdlib
    -fuse-ld=lld
    -Wl,-subsystem:efi_application
    -Wl,-entry:efi_main
)

add_dependencies(bootx64 kernel)

# Stage bootloader at standard UEFI fallback boot path
add_custom_command(TARGET bootx64 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ESP_DIR}/EFI/BOOT"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:bootx64>" "${ESP_DIR}/EFI/BOOT/BOOTX64.EFI"
    COMMENT "Deploying BOOTX64.EFI to EFI System Partition (esp/EFI/BOOT)..."
)

# ------------------------------------------------------------------------------
# QEMU / OVMF Emulation Target
# ------------------------------------------------------------------------------

find_program(QEMU_EXECUTABLE NAMES qemu-system-x86_64)
find_file(OVMF_BIOS
    NAMES edk2-x86_64-code.fd OVMF_CODE.fd OVMF.fd
    PATHS
        "C:/Program Files/qemu/share"
        "C:/Program Files (x86)/qemu/share"
        "/usr/share/OVMF"
        "/usr/share/ovmf"
        "/usr/share/edk2-ovmf/x64"
        "/opt/homebrew/share/qemu"
        "/usr/local/share/qemu"
)

if(QEMU_EXECUTABLE AND OVMF_BIOS)
    add_custom_target(run
        COMMAND "${QEMU_EXECUTABLE}"
            -drive if=pflash,format=raw,readonly=on,file=${OVMF_BIOS}
            -drive file=fat:rw:${ESP_DIR},format=raw
            -m 512M
            -net none
            -serial stdio
        DEPENDS bootx64
        USES_TERMINAL
        COMMENT "Running UEFI application and Kernel in QEMU..."
    )
endif()
