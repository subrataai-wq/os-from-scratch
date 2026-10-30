cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_TOOLCHAIN_FILE)
  message(FATAL_ERROR
    "Configure through the preset so the UEFI toolchain is used:\n"
    "    cmake --preset uefi\n"
    "    cmake --build --preset uefi")
endif()

project(myos LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- UEFI loader: becomes \EFI\BOOT\BOOTX64.EFI (spec 3.5.1.1) --------------
set(ESP_DIR "${CMAKE_BINARY_DIR}/esp")

add_executable(bootx64 main.c)
target_include_directories(bootx64 PRIVATE "${CMAKE_SOURCE_DIR}")
target_compile_options(bootx64 PRIVATE -Wall -Wextra)
set_target_properties(bootx64 PROPERTIES OUTPUT_NAME BOOTX64 SUFFIX ".EFI")

# Build a fake EFI System Partition inside the build tree after every build.
add_custom_command(TARGET bootx64 POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${ESP_DIR}/EFI/BOOT"
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:bootx64>" "${ESP_DIR}/EFI/BOOT/BOOTX64.EFI"
  VERBATIM)

# Anything in esp-extra/ is copied to the root of the ESP (e.g. hello.txt).
if(EXISTS "${CMAKE_SOURCE_DIR}/esp-extra")
  add_custom_command(TARGET bootx64 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/esp-extra" "${ESP_DIR}"
    VERBATIM)
endif()

# ---- QEMU run targets --------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(UefiQemu)
uefi_add_qemu_targets(bootx64 "${ESP_DIR}")