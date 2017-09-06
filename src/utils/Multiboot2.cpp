/**
 *   @file: Multiboot2.cpp
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#include "kstd.h"
#include "Multiboot2.h"

using namespace kstd;
namespace utils {

unsigned long long Multiboot2::multiboot2_info_addr;
unsigned int Multiboot2::multiboot2_info_totalsize;
BasicMemInfo* Multiboot2::bmi;
CommandLine* Multiboot2::cmd;
BootLoader* Multiboot2::bl;
FrameBuffer* Multiboot2::fb;
MemoryMap* Multiboot2::mm;
MemoryMapEntry* Multiboot2::mme[20];    // 20 is selected arbitrarily
unsigned int Multiboot2::mme_count;
Elf64Sections* Multiboot2::es;
Elf64SectionHeader* Multiboot2::esh[50];// 50 is selected arbitrarily
unsigned int Multiboot2::esh_count;


/**
 * @brief   Parse the multiboot2 structures stored by the boot loader at "multiboot2_info_ptr"
 *          This is necessary to eg. obtain memory range available for use
 */
void Multiboot2::initialize(void *multiboot2_info_ptr) {
    multiboot2_info_addr = (unsigned long long)multiboot2_info_ptr;
    char *tag_ptr = (char*)multiboot2_info_ptr;

    // read the always present Multiboot2 global tag
    GlobalTag *global = (GlobalTag*)tag_ptr;
    multiboot2_info_totalsize = global->total_size;
    tag_ptr += sizeof(GlobalTag);

    // read subsequent optional tags
    bool done = false;
    while (!done) {
        TagHeader *t = (TagHeader*) tag_ptr;

        switch (t->type) {
        case 0: {
            done = true;
            break;
        }

        case 1: {
            CommandLine *cmdp = (CommandLine*)tag_ptr;
            cmd = cmdp;
            break;
        }

        case 2: {
            BootLoader *blp = (BootLoader*)tag_ptr;
            bl = blp;
            break;
        }

        case 4: {
            BasicMemInfo *bmip = (BasicMemInfo*)tag_ptr;
            bmi = bmip;
            break;
        }

        case 6: {
            MemoryMap *mmp = (MemoryMap*)tag_ptr;
            mm = mmp;
            mme_count = (mm->size - sizeof(MemoryMap)) / mm->entry_size;

            char *entry_ptr = tag_ptr + sizeof(MemoryMap);
            for (int i = 0; i < mme_count; i++) {
                MemoryMapEntry *mmep = (MemoryMapEntry*)entry_ptr;
                mme[i] = mmep;
                entry_ptr += mm->entry_size;
            }

            break;
        }

        case 8: {
            FrameBuffer *fbp = (FrameBuffer*)tag_ptr;
            fb = fbp;
            break;
        }

        case 9: {
            Elf64Sections *esp = (Elf64Sections*)tag_ptr;
            es = esp;
            esh_count = es->num;

            char *entry_ptr = tag_ptr +  sizeof(Elf64Sections);
            for (int i = 0; i < esh_count; i++) {
                Elf64SectionHeader *eshp = (Elf64SectionHeader*)entry_ptr;
                esh[i] = eshp;
                entry_ptr += sizeof(Elf64SectionHeader);
            }

            break;
        }

        default:
            break;
        }

        tag_ptr+= (t->size+7)/8*8; // advance pointer by t->size, 8 byte aligned
    }
}

/**
 * @return  First byte of memory available for use
 */
size_t Multiboot2::get_available_memory_first_byte() {
    // seems that GRUB2 puts multiboot2 data after all elf sections, so let's return first byte after multiboot2 data
    return (size_t)multiboot2_info_addr + multiboot2_info_totalsize;
}

/**
 * @return  Last byte of memory available for use
 */
size_t Multiboot2::get_available_memory_last_byte() {
    return bmi->upper * 1024;
}

/**
 * @return  String representation of multiboot2 data
 */
string Multiboot2::to_string() {
    string result;

    result += format("boot loader: %, ", bl->name);
    result += format("boot cmdline: %\n", cmd->cmd);
    result += format("framebuffer: %x%x%, colors: %\n", fb->width, fb->height, fb->bpp, kstd::enum_to_str(fb->fb_type, "indexed=0x0"," RGB=0x1", "EGA text=0x2").c_str());
    result += format("memory info: lower: %KB, upper:%MB\n", bmi->lower, bmi->upper / 1024);
    result += format("memory map: size: %, entry size: %, entry version: %\n", mm->size, mm->entry_size, mm->entry_version);
    result += format("memory areas:\n");
    for (int i = 0; i < mme_count; i++) {
        kstd::string type =
                kstd::enum_to_str(mme[i]->type,
                        "Available=0x1",
                        "Reserved=0x2",
                        "ACPI reclaimable=0x3",
                        "ACPI NVS=0x4",
                        "BAD=0x5");

        result += format("   addr: %KB, len: %KB, type: %\n",
                    mme[i]->address / 1024, mme[i]->length / 1024, type.c_str());
    }

    result += format("elf sections: \n");

    for (int i = 0; i < esh_count; i++) {
        kstd::string type =
                kstd::enum_to_str(esh[i]->type,
                "NULL=0x0",
                "PROGBITS=0x1",
                "SYMTAB=0x2",
                "STRTAB=0x3",
                "RELA=0x4",
                "HASH=0x5",
                "DYNAMIC=0x6"
                "NOTE=0x7",
                ".BSS=0x8",
                "REL=0x9",
                "SHLIB=0x0A",
                "DYNSYM=0x0B"
                "CONSTRUCTORS=0x0E",
                "DESTRUCTORS=0x0F",
                "PRECONSTRUC=0x10",
                "GROUP=0x11",
                "SYMTAB_SHNDX=0x12",
                "NUM=0x13",
                "OS_SPECYFIC=0x60000000"
                );

        kstd::string flags =
                kstd::flags_to_str(esh[i]->flags,
                "WRITE=0x1",
                "ALLOC=0x2",
                "EXEC=0x4",
                "MERGE=0x10",
                "STRINGS=0x20",
                "INFO_LINK=0x40",
                "LINK_ORDER=0x80",
                "OS_NONCONFORMING=0x100"
                "GROUP=0x200",
                "TLS=0x400",
                "MASKOS=0x0FF00000",
                "MASKPROC=0xF0000000",
                "ORDERED=0x4000000",
                "EXCLUDE=0x8000000"
                );

        result += format("   addr: %, len: %, type: %, flags: %\n", esh[i]->addr, esh[i]->size, type.c_str(), flags.c_str());
    }


    result += format("multiboot: addr: %, len: %\n", multiboot2_info_addr, multiboot2_info_totalsize);

    return result;
}

} // namespace utils
