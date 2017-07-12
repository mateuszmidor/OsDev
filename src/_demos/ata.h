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

void print_volume_info(VolumeFat32& v) {
    printer.format("Label: %, Type: %, Size: %MB, Used: % clusters\n",
            v.get_label(),
            v.get_type(),
            v.get_size_in_bytes() / 1024 / 1024,
            v.get_used_space_in_clusters());
}

void print_file(VolumeFat32& v, string filename) {
    SimpleDentryFat32 file;
    if (!v.get_entry(filename, file)) {
        printer.format("File % not found\n", filename);
        return;
    }

    if (file.is_directory) {
        printer.format("% is a directory\n", filename);
        return;
    }

    const u32 SIZE = 513;
    char buff[SIZE];
    u32 count = v.read_file_entry(file, buff, SIZE-1);
    buff[count] = '\0';
    printer.format("%:\n", filename);
    printer.format("%\n", buff);
}

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

using OnTreeEntryFound = std::function<bool(const SimpleDentryFat32&, u8)>;
void traverse_tree(VolumeFat32& v, const SimpleDentryFat32& entry, u8 level, const OnTreeEntryFound& user_on_entry) {
    OnEntryFound on_entry = [&](const SimpleDentryFat32& e) -> bool {
        user_on_entry(e, level);
        if (e.is_directory && e.name != "." && e.name != "..")
            traverse_tree(v, e, level+1, user_on_entry);

        return true;
    };

    v.enumerate_directory_entry(entry, on_entry);
}

void print_tree_(VolumeFat32& v, const SimpleDentryFat32& e) {


}

void print_tree(VolumeFat32& v, string path) {
    SimpleDentryFat32 directory;
    v.get_entry(path, directory);

    auto on_entry = [&](const SimpleDentryFat32& e, u8 level) -> bool {
         if (e.name == "." || e.name == "..")
             return true;

         if (e.is_directory) {
             printer.format("%[%]\n", INDENTS[level], e.name);
         } else {
             if (e.size == 0)
                 printer.format("%% - %B\n", INDENTS[level], e.name, e.size);
             else
             {
                 const u32 SIZE = 33;
                 char buff[SIZE];
                 u32 count = v.read_file_entry(e, buff, SIZE-1);
                 buff[count-1] = '\0';
                 printer.format("%% - %B, %\n", INDENTS[level], e.name, e.size, buff);
             }
         }
         return true;
     };
    printer.format("%:\n", path);
    traverse_tree(v, directory, 1, on_entry);
}


void delete_tree_but(VolumeFat32& v, string root, string but) {
    SimpleDentryFat32 directory;
    v.get_entry(root, directory);

    auto on_entry = [&](const SimpleDentryFat32& e, u8 level) -> bool {
        v.delete_entry(e.name);
        return true;
    };

    traverse_tree(v, directory, 1, on_entry);
}

void print_hdd_info(AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd)) {
        printer.format("Not MBR formatted device\n");
        return;
    }

    MassStorageMsDos ms(hdd);

    //auto& volumes = ms.get_volumes();
    auto &v = ms.get_volumes()[1];
    //for (auto& v : volumes) {
        print_volume_info(v);
        for (int i = 1; i < 10; i++) {
            string name = "/FILE_";
            name.push_back(char('0' + i));
            v.delete_entry(name);
        }
//        v.delete_entry("/LEVEL1/LEVEL2/LEVEL3/LEVEL3.TXT");
//        v.delete_entry("/LEVEL1/LEVEL2/LEVEL3");
//        v.delete_entry("/LEVEL1/LEVEL2/LEVEL2.TXT");
//        v.delete_entry("/LEVEL1/LEVEL2");
//        v.delete_entry("/LEVEL1/LEVEL1.TXT");
        v.delete_entry("/TMP/TMP1.TXT");
        v.delete_entry("/TMP/TMP2.TXT");
        //v.delete_entry("/TMP");

        if (!v.create_entry("/TMP/first.txt", false))
            printer.format("Creating file failed\n");
        if (!v.create_entry("/TMP/second.txt", false))
            printer.format("Creating file failed\n");
        print_volume_info(v);
        print_tree(v, "/");
   // }
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
