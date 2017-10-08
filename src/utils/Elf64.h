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

/**
 * @brief   ELF64 file identification bytes
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf, Table 2. ELF Identification, e_ident
 */
enum Elf64_Ident {
    EI_MAG0         = 0,    // File identification
    EI_MAG1         = 1,    // File identification
    EI_MAG2         = 2,    // File identification
    EI_MAG3         = 3,    // File identification
    EI_CLASS        = 4,    // File class
    EI_DATA         = 5,    // Data encoding
    EI_VERSION      = 6,    // File version
    EI_OSABI        = 7,    // OS/ABI identification
    EI_ABIVERSION   = 8,    // ABI version
    EI_PAD          = 9,    // Start of padding  bytes
    EI_NIDENT       = 16    // sizeof e_ident[]
};

/**
 * @brief   ELF64 file header
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf, Figure 2. ELF-64 Header
 */
struct Elf64_Ehdr {
    unsigned char   e_ident[16];    // ELF identification
    Elf64_Half      e_type;         // Object file type
    Elf64_Half      e_machine;      // Machine type
    Elf64_Word      e_version;      // Object file version
    Elf64_Addr      e_entry;        // Entry point address
    Elf64_Off       e_phoff;        // Program header offset
    Elf64_Off       e_shoff;        // Section header offse
    Elf64_Word      e_flags;        // Processor-specific flags
    Elf64_Half      e_ehsize;       // ELF header size
    Elf64_Half      e_phentsize;    // Size of program header entry
    Elf64_Half      e_phnum;        // Number of program header entries
    Elf64_Half      e_shentsize;    // Size of section header entry
    Elf64_Half      e_shnum;        // Number of section header entries
    Elf64_Half      e_shstrndx;     //Section name string table index
} __attribute__((packed));

/**
 * @brief   ELF64 section header
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf, Figure 3. ELF-64 Section Header
 */
struct Elf64_Shdr {
    Elf64_Word      sh_name;        // Section  name offset in section header names table, see   Elf64_Ehdr::e_shstrndx
    Elf64_Word      sh_type;        // Section  type
    Elf64_Xword     sh_flags;       // Section  attributes; see Elf64_SHF
    Elf64_Addr      sh_addr;        // Virtual  address  in  memory
    Elf64_Off       sh_offset;      // Offset  in  file
    Elf64_Xword     sh_size;        // Size  of  section
    Elf64_Word      sh_link;        // Link  to  other  section
    Elf64_Word      sh_info;        // Miscellaneous  information
    Elf64_Xword     sh_addralign;   // Address  alignment  boundary
    Elf64_Xword     sh_entsize;     // Size  of  entries;  if  section  has  table
} __attribute__((packed));

/**
 * @brief   ELF64 section header flags
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf, Table 9. Section Attributes, sh_flags
 */
enum Elf64_SHF {
    WRITE   = 1,    // Section contains writable data
    ALLOC   = 2,    // Section is allocated in RAM
    EXEC    = 4     // Section contains executable instructions
};

/**
 * @brief   ELF64 program header
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf, Figure 6. ELF-64 Program Header Table Entry
 */
struct Elf64_Phdr {
    Elf64_Word      p_type;         //  Type  of  segment
    Elf64_Word      p_flags;        //  Segment  attributes
    Elf64_Off       p_offset;       //  Offset  in  file
    Elf64_Addr      p_vaddr;        //  Virtual  address  in  memory
    Elf64_Addr      p_paddr;        //  Reserved
    Elf64_Xword     p_filesz;       //  Size  of  segment  in  file
    Elf64_Xword     p_memsz;        //  Size  of  segment  in  memory
    Elf64_Xword     p_align;        //  Alignment  of  segment
} __attribute__((packed));

/**
 * @brief   This class allows loading of Executable Linkable Format files
 * @see     https://www.uclibc.org/docs/elf-64-gen.pdf
 */
class Elf64 {
public:
    static kstd::string to_string(void* elf64_data);
    static kstd::string section_header_to_string(const char* section_names, Elf64_Shdr* section_header);
    static kstd::string segment_header_to_string(Elf64_Phdr* segment_header);
    static u64 load_into_current_addressspace(void* elf64_data);
};

} /* namespace utils */

#endif /* SRC_UTILS_ELF64_H_ */
