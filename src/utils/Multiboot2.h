/**
 *   @file: Multiboot2.h
 *
 *   @date: Jun 6, 2017
 * @author: mateusz
 */

#ifndef SRC_MULTIBOOT2_H_
#define SRC_MULTIBOOT2_H_

#include <cstdint>
#include "kstd.h"
#include "Elf64.h"

namespace utils {

struct GlobalTag {
    unsigned int total_size;
    unsigned int reserved;
} __attribute__((packed));

struct TagHeader {
    unsigned int type;
    unsigned int size;
} __attribute__((packed));

struct BasicMemInfo {
    uint32_t type;  // = 4
    unsigned int size;
    unsigned int lower; // in KB
    unsigned int upper; // in KB
} __attribute__((packed));

struct MemoryMapEntry {
    unsigned long long address;  // physical
    unsigned long long length;
    unsigned int type;  // 1 = available, 2 = reserved, 3 = ACPI info, 4 = hibernation reserved, 5 = badram
    unsigned int reserved;
} __attribute__((packed));

struct MemoryMap {
    unsigned int type;  // = 6
    unsigned int size;
    unsigned int entry_size;
    unsigned int entry_version;
    MemoryMapEntry entries[0];
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
    unsigned char fb_type;  // indexed=0x0, RGB=0x1, EGA text=0x2
    unsigned char reserved; // = 0
    // here color palette if fb_type == 0
} __attribute__((packed));

struct Elf64Sections {
    unsigned int type;  // = 9
    unsigned int size;
    unsigned int num;
    unsigned int ent_size;
    unsigned int shndx; // index into below "headers". The pointed header contains section names C string at "addr"
    Elf64_Shdr headers[0];
} __attribute__((packed));



/**
 * @class   Multiboot2
 * @see     http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
 * @note    This class is specific as it is not meant to be instantiated, just used in a static-class way.
 *          This is because it must not rely on global constructors nor dynamic memory (it is used at the very beginning of kernel startup)
 */
class Multiboot2 {
public:
    static void initialize(void *multiboot2_info_ptr);
    static size_t get_available_memory_first_byte();
    static size_t get_available_memory_last_byte();
    static kstd::string to_string();

private:
    Multiboot2() = delete;  // don't instantiate this class

    static unsigned long long multiboot2_info_addr;
    static unsigned int multiboot2_info_totalsize;
    static BasicMemInfo* bmi;
    static CommandLine* cmd;
    static BootLoader* bl;
    static FrameBuffer* fb;
    static MemoryMap* mm;
    static MemoryMapEntry* mme[];
    static unsigned int mme_count;
    static Elf64Sections* es;
    static Elf64_Shdr* esh[];
    static unsigned int esh_count;
};

} // namespace utils

#endif /* SRC_MULTIBOOT2_H_ */
