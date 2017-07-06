/**
 *   @file: ata.h
 *
 *   @date: Jul 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_ATA_H_
#define SRC__DEMOS_ATA_H_

#include <memory>
#include "kstd.h"
#include "ScreenPrinter.h"
#include "AtaDriver.h"
#include "MassStorageMsDos.h"

namespace demos {

using namespace kstd;
using namespace drivers;
using namespace filesystem;

static ScreenPrinter& printer = ScreenPrinter::instance();

static const char* INDENTS[] = {
        "",
        "  ",
        "    ",
        "      ",
        "        ",
        "          ",
        "            ",
        "              ",
        "                ",
        "                  "};



void print_dir_tree(VolumeFat32& v, const SimpleDentryFat32& entry, u8 level)  {
    auto on_entry = [&](const SimpleDentryFat32& e) -> bool {
        if (e.name == "." || e.name == "..") // skip . and ..
            return true;

        if (e.is_directory) {
            printer.format("%[%]\n", INDENTS[level], e.name);
            print_dir_tree(v, e, level+1);
        } else
            printer.format("%% - %B\n", INDENTS[level], e.name, e.size);

        return true;
    };

    v.enumerate_directory(entry, on_entry);
}

void print_file(VolumeFat32& v, string filename) {
    SimpleDentryFat32 file;
    if (!v.get_entry_for_path(filename, file)) {
        printer.format("File % not found\n", filename);
        return;
    }

    if (file.is_directory) {
        printer.format("% is a directory\n", filename);
        return;
    }

    const u32 SIZE = 513;
    char buff[SIZE];
    u32 count = v.read_file(file, buff, SIZE-1);
    buff[count] = '\0';
    printer.format("%:\n", filename);
    printer.format("%\n", buff);
}

void print_tree(VolumeFat32& v, string path) {
    SimpleDentryFat32 directory;
    if (!v.get_entry_for_path(path, directory)) {
        printer.format("Directory % does not exist\n", path);
        return;
    }

    if (!directory.is_directory) {
        printer.format("% is not a directory\n", path);
        return;
    }
    printer.format("%:\n", path);
    print_dir_tree(v, directory, 1);
}


void print_hdd_info(AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd)) {
        printer.format("Not MBR formatted device\n");
        return;
    }

    MassStorageMsDos ms(hdd);
    auto& volumes = ms.get_volumes();
    for (auto& v : volumes) {
        v.print_volume_info(printer);
        print_tree(v, "/TMP");
        print_file(v, "/TO_BE_~1");
    }
}

void ata_demo(std::shared_ptr<AtaPrimaryBusDriver> ata_primary_bus) {
    if (ata_primary_bus->master_hdd.is_present()) {
        printer.format("ATA Primary Master: present\n");
        print_hdd_info(ata_primary_bus->master_hdd);
    } else
        printer.format("ATA Primary Master: not present\n");

    if (ata_primary_bus->slave_hdd.is_present()) {
        printer.format("ATA Primary Slave: present\n");
        print_hdd_info(ata_primary_bus->slave_hdd);
    } else
        printer.format("ATA Primary Slave: not present\n");
}

}

#endif /* SRC__DEMOS_ATA_H_ */
