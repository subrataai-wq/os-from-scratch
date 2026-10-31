/*
 * elf64.h - Executable and Linkable Format (ELF-64) definitions.
 *
 * Conforms to System V Application Binary Interface & ELF-64 Object File Format.
 */

#ifndef _ELF64_H_
#define _ELF64_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ELF_MAGIC
#define ELF_MAGIC 0x464C457FU /* 0x7F, 'E', 'L', 'F' */
#endif

#ifndef PT_LOAD
#define PT_LOAD 1U
#endif

typedef struct
{
  uint8_t  e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
} Elf64_Phdr;

/*
 * Static assertions to verify ABI structure size invariants.
 */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define _ELF64_SA(c, m) static_assert(c, m)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define _ELF64_SA(c, m) _Static_assert(c, m)
#else
#define _ELF64_SA(c, m) extern char _elf64_static_assert_dummy[(c) ? 1 : -1]
#endif

_ELF64_SA(sizeof(Elf64_Ehdr) == 64, "Elf64_Ehdr size must be 64 bytes");
_ELF64_SA(sizeof(Elf64_Phdr) == 56, "Elf64_Phdr size must be 56 bytes");

#ifdef __cplusplus
}
#endif

#endif /* _ELF64_H_ */

