/**
 *   @file: Elf64.h
 *
 *   @date: Sep 7, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_ELF64_H_
#define SRC_UTILS_ELF64_H_

#include <cstdint>
#include "kstd.h"

namespace utils {

typedef uint64_t    Elf64_Addr;
typedef uint64_t    Elf64_Off;
typedef uint16_t    Elf64_Half;
typedef uint32_t    Elf64_Word;
typedef int32_t     Elf64_Sword;
typedef uint64_t    Elf64_Xword;
typedef int64_t     Elf64_Sxword;

enum Identification {
    EI_MAG0         = 0,
    EI_MAG1         = 1,
    EI_MAG2         = 2,
    EI_MAG3         = 3,
    EI_CLASS        = 4,
    EI_DATA         = 5,
    EI_VERSION      = 6,
    EI_OSABI        = 7,
    EI_ABIVERSION   = 8,
    EI_PAD          = 9,
    EI_NIDENT       = 16
};

struct bfelf_ehdr {
    unsigned char   e_ident[Identification::EI_NIDENT];
    Elf64_Half      e_type;
    Elf64_Half      e_machine;
    Elf64_Word      e_version;
    Elf64_Addr      e_entry;
    Elf64_Off       e_phoff;
    Elf64_Off       e_shoff;
    Elf64_Word      e_flags;
    Elf64_Half      e_ehsize;
    Elf64_Half      e_phentsize;
    Elf64_Half      e_phnum;
    Elf64_Half      e_shentsize;
    Elf64_Half      e_shnum;
    Elf64_Half      e_shstrndx;
};

struct Elf64_Shdr {
    Elf64_Word      sh_name;        /*  Section  name  */
    Elf64_Word      sh_type;        /*  Section  type  */
    Elf64_Xword     sh_flags;       /*  Section  attributes  */
    Elf64_Addr      sh_addr;        /*  Virtual  address  in  memory  */
    Elf64_Off       sh_offset;      /*  Offset  in  file  */
    Elf64_Xword     sh_size;        /*  Size  of  section  */
    Elf64_Word      sh_link;        /*  Link  to  other  section  */
    Elf64_Word      sh_info;        /*  Miscellaneous  information  */
    Elf64_Xword     sh_addralign;   /*  Address  alignment  boundary  */
    Elf64_Xword     sh_entsize;     /*  Size  of  entries;  if  section  has  table  */
} __attribute__((packed));;

/**
 * @brief   This class allows loading of Executable Linkable Format files
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf
 */
class Elf64 {
public:
    kstd::string to_string(void* elf64_data) const;
};

} /* namespace utils */

#endif /* SRC_UTILS_ELF64_H_ */
