/**
 *   @file: Multiboot2.cpp
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#include "Multiboot2.h"

Multiboot2::Multiboot2(void *ebx) {
    // skip the global tag that comes first and is always present
    char *tag_ptr = (char*)ebx + sizeof(GlobalTag);

    // read subsequent tags
    bool done = false;
    while (!done) {
        TagHeader * t = (TagHeader*) tag_ptr;

        switch (t->type) {
        case 0: {
            done = true;
            break;
        }

        case 1: {
            CommandLine * cmdp = (CommandLine*)tag_ptr;
            cmd = *cmdp;
            break;
        }

        case 2: {
            BootLoader * blp = (BootLoader*)tag_ptr;
            bl = *blp;
            break;
        }

        case 4: {
            BasicMemInfo * bmip = (BasicMemInfo*)tag_ptr;
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

        default:
            break;
        }

        tag_ptr+= (t->size+7)/8*8; // advance pointer by t->size, 8 byte aligned
    }
}

void Multiboot2::print(ScreenPrinter &p) {
    p.format("boot loader: %\n", bl.name);
    p.format("boot cmdline: %\n", cmd.cmd);
    p.format("framebuffer %x%x%, colors: %\n", fb.width, fb.height, fb.bpp, fb.fb_type == 0 ? "indexed" : "non indexed");
    p.format("memory info: lower: %KB, upper:%MB\n", bmi.type, bmi.size, bmi.lower, bmi.upper / 1024);
    p.format("memory map: size: %, entry size: %, entry version: %\n", mm.size, mm.entry_size, mm.entry_version);
    p.format("memory areas:\n");
    for (int i = 0; i < mme_count; i++)
        p.format("   addr: %KB, len: %KB, type: %(%) \n",
                mme[i].address / 1024, mme[i].length / 1024, mme[i].type, mme[i].type == 1 ? "available" : "reserved");
}
