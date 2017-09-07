/**
 *   @file: Elf64.cpp
 *
 *   @date: Sep 7, 2017
 * @author: Mateusz Midor
 */

#include "Elf64.h"

using namespace kstd;
namespace utils {

string Elf64::to_string(void* elf64_data) const {
    bfelf_ehdr* hdr = (bfelf_ehdr*) elf64_data;

    string result;
    result += enum_to_str(hdr->e_ident[Identification::EI_CLASS], "32bit=0x1", "64bit=0x2") + "\n";
    result += enum_to_str(hdr->e_ident[Identification::EI_OSABI], "SYSV=0x0", "HPUX=0x1", "STANDALONE=0xFF") + "\n";
    result += enum_to_str(hdr->e_type,
            "No file type=0x0",
            "Relocatable=0x1",
            "Executable=0x2",
            "Shared object=0x3",
            "Core file=0x4") + "\n";

    return result;
}

} /* namespace utils */
