/**
 *   @file: Multiboot2.h
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#ifndef SRC_MULTIBOOT2_H_
#define SRC_MULTIBOOT2_H_

#include "ScreenPrinter.h"

struct GlobalTag {
    unsigned int total_size;
    unsigned int reserved;
} __attribute__((packed));

struct TagHeader {
    unsigned int type;
    unsigned int size;
} __attribute__((packed));

struct BasicMemInfo {
    unsigned int type;  // = 4
    unsigned int size;
    unsigned int lower;
    unsigned int upper;
} __attribute__((packed));

struct MemoryMap {
    unsigned int type;  // = 6
    unsigned int size;
    unsigned int entry_size;
    unsigned int entry_version;
} __attribute__((packed));

struct MemoryMapEntry {
    unsigned long long address;  // physical
    unsigned long long length;
    unsigned int type;  // 0 = reserved, 1 = available, 3 = ACPI info, 4 = hibernation reserved
    unsigned int reserved;
} __attribute__((packed));

struct CommandLine {
    unsigned int type;  // = 1
    unsigned int size;
    char cmd[32];
} __attribute__((packed));

struct BootLoader {
    unsigned int type;  // = 2
    unsigned int size;
    char name[32];
} __attribute__((packed));

struct FrameBuffer {
    unsigned int type;  // = 8
    unsigned int size;
    unsigned long long address; // should be below 4GiB
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
    unsigned char bpp;      // bits per pixel
    unsigned char fb_type;  // indexed color if = 0
    unsigned char reserved; // = 0
    // here color palette if fb_type == 0
} __attribute__((packed));

/**
 * @class   Multiboot2
 * @see     http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
 */
class Multiboot2 {
public:
    Multiboot2(void *multiboot2_info_ptr);
    void print(ScreenPrinter &p);

private:

    BasicMemInfo bmi;
    CommandLine cmd;
    BootLoader bl;
    FrameBuffer fb;
    MemoryMap mm;
    MemoryMapEntry mme[20]; // 20 is selected arbitrarily
    unsigned int mme_count;
};


#endif /* SRC_MULTIBOOT2_H_ */
