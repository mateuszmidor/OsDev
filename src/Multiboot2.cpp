/**
 *   @file: Multiboot2.cpp
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#include "Multiboot2.h"

Multiboot2::Multiboot2(void *multiboot2_info_ptr) {
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
            cmd = *cmdp;
            break;
        }

        case 2: {
            BootLoader *blp = (BootLoader*)tag_ptr;
            bl = *blp;
            break;
        }

        case 4: {
            BasicMemInfo *bmip = (BasicMemInfo*)tag_ptr;
            bmi = *bmip;
            break;
        }

        case 6: {
            MemoryMap *mmp = (MemoryMap*)tag_ptr;
            mm = *mmp;
            mme_count = (mm.size - sizeof(MemoryMap)) / mm.entry_size;

            char *entry_ptr = tag_ptr + sizeof(MemoryMap);
            for (int i = 0; i < mme_count; i++) {
                MemoryMapEntry *mmep = (MemoryMapEntry*)entry_ptr;
                mme[i] = *mmep;
                entry_ptr += mm.entry_size;
            }

            break;
        }

        case 8: {
            FrameBuffer *fbp = (FrameBuffer*)tag_ptr;
            fb = *fbp;
            break;
        }

        case 9: {
            Elf64Sections *esp = (Elf64Sections*)tag_ptr;
            es = *esp;
            esh_count = es.num;

            char *entry_ptr = tag_ptr +  sizeof(Elf64Sections);
            for (int i = 0; i < esh_count; i++) {
                Elf64SectionHeader *eshp = (Elf64SectionHeader*)entry_ptr;
                esh[i] = *eshp;
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

void Multiboot2::print(ScreenPrinter &p) {
    p.format("boot loader: %, ", bl.name);
    p.format("boot cmdline: %\n", cmd.cmd);
    p.format("framebuffer: %x%x%, colors: %\n", fb.width, fb.height, fb.bpp, fb.fb_type == 0 ? "indexed" : "non indexed");
    p.format("memory info: lower: %KB, upper:%MB\n", bmi.lower, bmi.upper / 1024);
//    p.format("memory map: size: %, entry size: %, entry version: %\n", mm.size, mm.entry_size, mm.entry_version);
    p.format("memory areas:\n");
    for (int i = 0; i < mme_count; i++)
        p.format("   addr: %KB, len: %KB, type: %(%) \n",
                    mme[i].address / 1024, mme[i].length / 1024, mme[i].type, mme[i].type == 1 ? "available" : "reserved");


    p.format("elf sections: \n");
    for (int i = 1; i < esh_count; i++)
        p.format("   addr: %, len: %, type: %, flags: %\n", esh[i].addr, esh[i].size, esh[i].type, esh[i].flags);

    p.format("multiboot: addr: %, len: %\n", multiboot2_info_addr, multiboot2_info_totalsize);
}
