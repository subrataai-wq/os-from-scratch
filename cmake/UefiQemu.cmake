# UefiQemu.cmake - locate QEMU and edk2/OVMF firmware on Windows, Linux and macOS,
# then add `run` and `run-debug` targets.
#
# Usage:  uefi_add_qemu_targets(<target> <esp_dir>)
#
# Override any detection at configure time, for example:
#   cmake --preset uefi -DQEMU_SYSTEM_X86_64=/path/to/qemu-system-x86_64
#                       -DQEMU_CODE_FD=/path/to/OVMF_CODE.fd -DQEMU_ACCEL=tcg

set(QEMU_ACCEL "auto" CACHE STRING "QEMU accelerator: auto, tcg, kvm, hvf, whpx")
set_property(CACHE QEMU_ACCEL PROPERTY STRINGS auto tcg kvm hvf whpx)
set(QEMU_MEMORY "256M" CACHE STRING "Guest RAM size")
option(QEMU_HEADLESS "Run without a window (serial output goes to this terminal)" OFF)

function(uefi_add_qemu_targets TARGET ESP_DIR)
  # ---- 1. QEMU executable ------------------------------------------------
  set(_hints)
  if(DEFINED ENV{QEMU_DIR})
    list(APPEND _hints "$ENV{QEMU_DIR}")
  endif()
  if(WIN32)
    list(APPEND _hints "C:/Program Files/qemu" "$ENV{ProgramFiles}/qemu")
  endif()
  list(APPEND _hints "/opt/homebrew/bin" "/usr/local/bin" "/usr/bin")
  find_program(QEMU_SYSTEM_X86_64 NAMES qemu-system-x86_64 HINTS ${_hints})
  if(NOT QEMU_SYSTEM_X86_64)
    message(WARNING "qemu-system-x86_64 not found: run targets disabled "
                    "(set QEMU_SYSTEM_X86_64 or the QEMU_DIR environment variable).")
    return()
  endif()
  get_filename_component(_qdir "${QEMU_SYSTEM_X86_64}" DIRECTORY)

  # ---- 2. UEFI firmware (code image, plus variable store when split) -----
  set(_fw_dirs
    "${_qdir}/share" "${_qdir}/../share/qemu" "${_qdir}/../share"
    "/usr/share/OVMF" "/usr/share/edk2/ovmf" "/usr/share/edk2/x64"
    "/usr/share/edk2-ovmf/x64" "/usr/share/qemu" "/usr/share/ovmf"
    "/opt/homebrew/share/qemu" "/usr/local/share/qemu")
  find_file(QEMU_CODE_FD
    NAMES edk2-x86_64-code.fd OVMF_CODE_4M.fd OVMF_CODE.4m.fd OVMF_CODE.fd OVMF.fd
    HINTS ${_fw_dirs})
  if(NOT QEMU_CODE_FD)
    message(WARNING "UEFI firmware not found: run targets disabled "
                    "(set QEMU_CODE_FD to your edk2/OVMF code file).")
    return()
  endif()

  get_filename_component(_code_name "${QEMU_CODE_FD}" NAME)
  get_filename_component(_code_dir  "${QEMU_CODE_FD}" DIRECTORY)
  if(_code_name STREQUAL "edk2-x86_64-code.fd")
    set(_vars_name "edk2-i386-vars.fd")            # QEMU's own pairing
  elseif(_code_name MATCHES "CODE")
    string(REPLACE "CODE" "VARS" _vars_name "${_code_name}")
  else()
    set(_vars_name "")                              # combined image such as OVMF.fd
  endif()
  if(_vars_name)
    find_file(QEMU_VARS_FD NAMES ${_vars_name} HINTS "${_code_dir}" NO_DEFAULT_PATH)
  endif()

  if(QEMU_VARS_FD)
    # Writable per-build copy, so UEFI variables persist like on real hardware.
    set(_vars_rw "${CMAKE_BINARY_DIR}/vars.fd")
    if(NOT EXISTS "${_vars_rw}")
      configure_file("${QEMU_VARS_FD}" "${_vars_rw}" COPYONLY)
    endif()
    set(_fw_args
      -drive "if=pflash,format=raw,readonly=on,file=${QEMU_CODE_FD}"
      -drive "if=pflash,format=raw,file=${_vars_rw}")
  else()
    message(WARNING "No matching variable store found; falling back to -bios.")
    set(_fw_args -bios "${QEMU_CODE_FD}")
  endif()

  # ---- 3. Accelerator ----------------------------------------------------
  string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _host_cpu)
  set(_accel "tcg")                                 # safe software emulation
  if(QEMU_ACCEL STREQUAL "auto")
    if(_host_cpu MATCHES "^(x86_64|amd64)$")        # only x86 hosts can accelerate x86 guests
      if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "/dev/kvm")
        set(_accel "kvm")
      elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(_accel "hvf")
      endif()
      # Windows stays on tcg: WHPX needs the Windows Hypervisor Platform feature,
      # so it is opt-in with -DQEMU_ACCEL=whpx.
    endif()
  else()
    set(_accel "${QEMU_ACCEL}")
  endif()
  if(_accel STREQUAL "kvm" OR _accel STREQUAL "hvf")
    set(_accel_args -accel ${_accel} -cpu host)
  elseif(_accel STREQUAL "whpx")
    set(_accel_args -accel whpx,kernel-irqchip=off)
  else()
    set(_accel_args -accel tcg)
  endif()

  # ---- 4. Assemble the command line and add targets ----------------------
  set(_args
    -machine q35 -m ${QEMU_MEMORY}
    ${_accel_args}
    ${_fw_args}
    -drive "format=raw,file=fat:rw:${ESP_DIR}"
    -nic none
    -serial stdio)
  if(QEMU_HEADLESS)
    list(APPEND _args -display none)
  endif()

  add_custom_target(run
    COMMAND "${QEMU_SYSTEM_X86_64}" ${_args}
    DEPENDS ${TARGET} USES_TERMINAL VERBATIM
    COMMENT "Booting ${ESP_DIR} in QEMU (${_accel})")
  add_custom_target(run-debug
    COMMAND "${QEMU_SYSTEM_X86_64}" ${_args} -s -S
    DEPENDS ${TARGET} USES_TERMINAL VERBATIM
    COMMENT "QEMU paused, GDB server on tcp::1234")

  message(STATUS "QEMU     : ${QEMU_SYSTEM_X86_64}")
  message(STATUS "Firmware : ${QEMU_CODE_FD}")
  message(STATUS "Vars     : ${QEMU_VARS_FD}")
  message(STATUS "Accel    : ${_accel}")
endfunction()