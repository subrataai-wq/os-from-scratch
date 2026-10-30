# Cross-compile toolchain: x86_64 UEFI application (PE32+, spec section 2.1.1).
# Works from a Windows, Linux or macOS host. Needs: clang, lld (lld-link), llvm-ar.

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Where LLVM tools commonly live. Override with the LLVM_DIR environment variable.
set(_llvm_hints)
if(DEFINED ENV{LLVM_DIR})
  list(APPEND _llvm_hints "$ENV{LLVM_DIR}/bin")
endif()
list(APPEND _llvm_hints
  "C:/msys64/ucrt64/bin"                 # MSYS2 UCRT64
  "C:/Program Files/LLVM/bin"            # official Windows installer
  "/opt/homebrew/opt/llvm/bin"           # macOS (Apple Silicon) Homebrew
  "/opt/homebrew/opt/lld/bin"
  "/usr/local/opt/llvm/bin"              # macOS (Intel) Homebrew
  "/usr/local/opt/lld/bin")

find_program(_clang NAMES clang HINTS ${_llvm_hints})
if(NOT _clang)
  message(FATAL_ERROR "clang not found. Install LLVM or set the LLVM_DIR environment variable.")
endif()
set(CMAKE_C_COMPILER "${_clang}")

# Archiver for CMake's compiler check (the host 'ar' may be missing or wrong).
find_program(_llvm_ar     NAMES llvm-ar     HINTS ${_llvm_hints})
find_program(_llvm_ranlib NAMES llvm-ranlib HINTS ${_llvm_hints})
if(_llvm_ar)
  set(CMAKE_AR "${_llvm_ar}")
endif()
if(_llvm_ranlib)
  set(CMAKE_RANLIB "${_llvm_ranlib}")
endif()

# We cannot run a UEFI image on the host, so only build a library to test the compiler.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# --target selects a Windows-style PE/COFF image, which is what UEFI loads.
# It is passed explicitly (not via CMAKE_C_COMPILER_TARGET) so that it reaches the
# link command too; without it MSYS2's clang defaults to the MinGW target and
# rejects -fuse-ld=lld-link.
set(CMAKE_C_FLAGS_INIT
  "--target=x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector")

# -fuse-ld=lld-link makes clang call lld-link on every host.
# Dash-style flags avoid the MSYS/Git-Bash '/flag' path mangling.
set(CMAKE_EXE_LINKER_FLAGS_INIT
  "-fuse-ld=lld-link -nostdlib -Wl,-subsystem:efi_application -Wl,-entry:efi_main -Wl,-nodefaultlib")