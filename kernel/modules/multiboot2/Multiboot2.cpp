/**
 *   @file: Multiboot2.cpp
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#include "kstd.h"
#include "Multiboot2.h"
#include "StringUtils.h"
#include "HigherHalf.h"


using namespace cstd;
using namespace memory;

namespace utils {

size_t Multiboot2::multiboot2_info_addr;
size_t Multiboot2::multiboot2_info_totalsize;
BasicMemInfo* Multiboot2::bmi;
CommandLine* Multiboot2::cmd;
BootLoader* Multiboot2::bl;
FrameBuffer* Multiboot2::fb;
MemoryMap* Multiboot2::mm;
MemoryMapEntry* Multiboot2::mme[20];    // 20 is selected arbitrarily
unsigned int Multiboot2::mme_count;
Elf64Sections* Multiboot2::es;
Elf64_Shdr* Multiboot2::esh[50];// 50 is selected arbitrarily
unsigned int Multiboot2::esh_count;


/**
 * @brief   Parse the multiboot2 structures stored by the boot loader at "multiboot2_info_ptr"
 *          This is necessary to eg. obtain memory range available for use
 */
void Multiboot2::initialize(void *multiboot2_info_ptr) {
    multiboot2_info_addr = (size_t)multiboot2_info_ptr;
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
                Elf64_Shdr *eshp = (Elf64_Shdr*)entry_ptr;

                // sh_addr is given as physical address. Need to convert it to higher half kernel virtual address so that it can be accessed
                eshp->sh_addr = HigherHalf::phys_to_virt(eshp->sh_addr);
                esh[i] = eshp;
                entry_ptr += sizeof(Elf64_Shdr);
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
 * @return  First byte of physical memory available for use
 */
size_t Multiboot2::get_available_memory_first_byte() {
    size_t first_free_byte = HigherHalf::phys_to_virt(bmi->lower * 1024);   // convert physical to virtual; elf sections use virtual

    for (u32 i = 0; i < esh_count; i++) {
        if ((esh[i]->sh_flags & Elf64_SHF::ALLOC) != Elf64_SHF::ALLOC) // consider section only if it has flag ALLOC, meaning it resides in ram
            continue;

        size_t last_occupied_byte = esh[i]->sh_addr + esh[i]->sh_size;
        if (first_free_byte <= last_occupied_byte)
            first_free_byte = last_occupied_byte + 1;
    }

    if (first_free_byte <= multiboot2_info_addr + multiboot2_info_totalsize)
        first_free_byte = multiboot2_info_addr + multiboot2_info_totalsize + 1;

    return HigherHalf::virt_to_phys(first_free_byte);   // return physical address
}

/**
 * @return  Last byte of physical memory available for use
 */
size_t Multiboot2::get_available_memory_last_byte() {
    return bmi->upper * 1024 ;
}

/**
 * @return  String representation of multiboot2 data
 */
string Multiboot2::to_string() {
    string result;

    result += StringUtils::format("boot loader: %, ", bl->name);
    result += StringUtils::format("boot cmdline: %\n", cmd->cmd);
    result += StringUtils::format("framebuffer: %x%x%, colors: %\n", fb->width, fb->height, fb->bpp, StringUtils::enum_to_str(fb->fb_type, "indexed=0x0"," RGB=0x1", "EGA text=0x2").c_str());
    result += StringUtils::format("memory info: lower: %KB, upper:%MB\n", bmi->lower, bmi->upper / 1024);
    result += StringUtils::format("memory map: size: %, entry size: %, entry version: %\n", mm->size, mm->entry_size, mm->entry_version);
    result += StringUtils::format("memory areas:\n");
    for (int i = 0; i < mme_count; i++) {
        string type =
            StringUtils::enum_to_str(mme[i]->type,
                    "Available=0x1",
                    "Reserved=0x2",
                    "ACPI reclaimable=0x3",
                    "ACPI NVS=0x4",
                    "BAD=0x5");

        result += StringUtils::format("   addr: %KB, len: %KB, type: %\n",
                    mme[i]->address / 1024, mme[i]->length / 1024, type.c_str());
    }

    result += StringUtils::format("elf sections: \n");
    char* section_names = (char*)es->headers[es->shndx].sh_addr;
    for (int i = 0; i < esh_count; i++) {
        string esh_str = Elf64::section_header_to_string(section_names, (Elf64_Shdr*)esh[i]);
        result += StringUtils::format("  %. %\n", i, esh_str);
    }

    result += StringUtils::format("multiboot: addr: %, len: %\n", multiboot2_info_addr, multiboot2_info_totalsize);

    return result;
}

} // namespace utils
