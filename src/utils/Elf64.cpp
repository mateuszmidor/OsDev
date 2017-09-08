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
    Elf64_Ehdr* hdr = (Elf64_Ehdr*) elf64_data;
    string result;

    result += format("ELF header size: %\n", hdr->e_ehsize);
    string MAGIC { (s8)hdr->e_ident[Elf64_Ident::EI_MAG1], (s8)hdr->e_ident[Elf64_Ident::EI_MAG2], (s8)hdr->e_ident[Elf64_Ident::EI_MAG3]};
    result += MAGIC  + "\n";
    result += enum_to_str(hdr->e_ident[Elf64_Ident::EI_CLASS], "32bit=0x1", "64bit=0x2") + "\n";
    result += enum_to_str(hdr->e_ident[Elf64_Ident::EI_DATA], "Little endian=0x1", "Big endian=0x2") + "\n";
    result += enum_to_str(hdr->e_ident[Elf64_Ident::EI_VERSION], "Version 1=0x1") + "\n";
    result += enum_to_str(hdr->e_ident[Elf64_Ident::EI_OSABI], "SYSV=0x0", "HPUX=0x1", "STANDALONE=0xFF") + "\n";
    result += enum_to_str(hdr->e_ident[Elf64_Ident::EI_ABIVERSION], "SYSVR3=0x0") + "\n";
    result += enum_to_str(hdr->e_type, "No file type=0x0", "Relocatable=0x1", "Executable=0x2", "Shared object=0x3", "Core file=0x4") + "\n";
    result += format("Entry point at virt address: %\n", hdr->e_entry);
    result += format("Program header table file offset: %\n", hdr->e_phoff);
    result += format("Program header table entry size: %\n", hdr->e_phentsize);
    result += format("Program header table entry count: %\n", hdr->e_phnum);
    result += format("Section header table file offset: %\n", hdr->e_shoff);
    result += format("Section header table entry size: %\n", hdr->e_shentsize);
    result += format("Section header table entry count: %\n", hdr->e_shnum);
    result += format("String table section index: %\n", hdr->e_shstrndx);

    result += format("elf sections: \n");

    Elf64_Shdr* sections_start = (Elf64_Shdr*)((char*)elf64_data + hdr->e_shoff);
    Elf64_Shdr* esh = sections_start;
    const char* section_names = (char*) ((char*)elf64_data + (sections_start + hdr->e_shstrndx)->sh_offset );
    for (int i = 0; i < hdr->e_shnum; i++) {
        string esh_str = section_header_to_string(section_names, esh);
        result += format("  %. %\n", i, esh_str);
        esh++;
    }

    return result;
}

string Elf64::section_header_to_string(const char* section_names, Elf64_Shdr* esh) {
    kstd::string type =
       enum_to_str(esh->sh_type,
           "NULL=0x0",
           "PROGBITS=0x1",
           "SYMTAB=0x2",
           "STRTAB=0x3",
           "RELA=0x4",
           "HASH=0x5",
           "DYNAMIC=0x6",
           "NOTE=0x7",
           "NO BITS=0x8",
           "REL=0x9",
           "SHLIB=0x0A",
           "DYNSYM=0x0B",
           "CONSTRUCTORS=0x0E",
           "DESTRUCTORS=0x0F",
           "PRECONSTRUC=0x10",
           "GROUP=0x11",
           "SYMTAB_SHNDX=0x12",
           "NUM=0x13",
           "OS_SPECYFIC=0x60000000"
       );

   kstd::string flags =
       flags_to_str(esh->sh_flags,
           "WRITE=0x1",
           "ALLOC=0x2",
           "EXEC=0x4",
           "MERGE=0x10",
           "STRINGS=0x20",
           "INFO_LINK=0x40",
           "LINK_ORDER=0x80",
           "OS_NONCONFORMING=0x100",
           "GROUP=0x200",
           "TLS=0x400",
           "MASKOS=0x0FF00000",
           "MASKPROC=0xF0000000",
           "ORDERED=0x4000000",
           "EXCLUDE=0x8000000"
       );


    u16 name_offset = esh->sh_name;
    const char* section_name = section_names + name_offset;
    return format("addr: %, len: %, name: %, type: %, flags: %", esh->sh_addr, esh->sh_size, section_name, type.c_str(), flags.c_str());
}

} /* namespace utils */
