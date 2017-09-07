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

typedef uint64_t    bfelf64_addr;
typedef uint64_t    bfelf64_off;
typedef uint16_t    bfelf64_half;
typedef uint32_t    bfelf64_word;
typedef int32_t     bfelf64_sword;
typedef uint64_t    bfelf64_xword;
typedef int64_t     bfelf64_sxword;

struct bfelf_ehdr {
    unsigned char   e_ident[16];
    bfelf64_half    e_type;
    bfelf64_half    e_machine;
    bfelf64_word    e_version;
    bfelf64_addr    e_entry;
    bfelf64_off     e_phoff;
    bfelf64_off     e_shoff;
    bfelf64_word    e_flags;
    bfelf64_half    e_ehsize;
    bfelf64_half    e_phentsize;
    bfelf64_half    e_phnum;
    bfelf64_half    e_shentsize;
    bfelf64_half    e_shnum;
    bfelf64_half    e_shstrndx;
};

enum Identification {
    EI_MAG0 = 0,
    EI_MAG1 = 1,
    EI_MAG2 = 2,
    EI_MAG3 = 3,
    EI_CLASS = 4,
    EI_DATA = 5,
    EI_VERSION = 6,
    EI_OSABI = 7,
    EI_ABIVERSION = 8,
    EI_PAD = 9,
    EI_NIDENT = 16
};
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
