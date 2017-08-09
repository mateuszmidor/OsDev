/*
 * Fat32Demo.cpp
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#include "Fat32Demo.h"
#include "VolumeFat32.h"
#include "MassStorageMsDos.h"
#include "KernelLog.h"
#include "DriverManager.h"

using namespace kstd;
using namespace drivers;
using namespace filesystem;
using utils::KernelLog;

namespace demos {

Fat32Demo::Fat32Demo() : klog(KernelLog::instance()) {
}

void Fat32Demo::run(u64 arg) {
    auto& driver_manager = DriverManager::instance();
    auto ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>();
    if (!ata_primary_bus) {
        klog.format("Fat32Demo::run: no AtaPrimaryBusDriver\n");
        return;
    }

    if (ata_primary_bus->master_hdd.is_present()) {
        klog.format("ATA Primary Master: present\n");
        print_hdd_info(ata_primary_bus->master_hdd);
    } else
        klog.format("ATA Primary Master: not present\n");

    if (ata_primary_bus->slave_hdd.is_present()) {
        klog.format("ATA Primary Slave: present\n");
        print_hdd_info(ata_primary_bus->slave_hdd);
    } else
        klog.format("ATA Primary Slave: not present\n");
}

void Fat32Demo::print_volume_info(VolumeFat32& v) {
    klog.format("Label: %, Type: %, Size: %MB, Used: % clusters\n",
                v.get_label(),
                v.get_type(),
                v.get_size_in_bytes() / 1024 / 1024,
                v.get_used_space_in_clusters());
}

void Fat32Demo::print_file(VolumeFat32& v, string filename) {
    Fat32Entry file = v.get_entry(filename);
    if (!file) {
        klog.format("File % not found\n", filename);
        return;
    }

    if (file.is_directory) {
        klog.format("% is a directory\n", filename);
        return;
    }

    const u32 SIZE = 513;
    char buff[SIZE];
    u32 count = file.read(buff, SIZE-1);
    buff[count] = '\0';
    klog.format("%:\n", filename);
    klog.format("%\n", buff);
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

void Fat32Demo::traverse_tree(VolumeFat32& v, const Fat32Entry& entry, u8 level, const OnTreeEntryFound& user_on_entry) {
    OnEntryFound on_entry = [&](Fat32Entry& e) -> bool {
        user_on_entry(e, level);
        if (e.is_directory && e.name != "." && e.name != "..")
            traverse_tree(v, e, level+1, user_on_entry);

        return true;
    };

    entry.enumerate_entries(on_entry);
}

void Fat32Demo::print_tree(VolumeFat32& v, string path) {
    Fat32Entry directory = v.get_entry(path);

    auto on_entry = [&](Fat32Entry& e, u8 level) -> bool {
         if (e.name == "." || e.name == "..")
             return true;

         if (e.is_directory) {
             klog.format("%[%]\n", INDENTS[level], e.name);
         } else {
             if (e.data.get_size() == 0)
                 klog.format("%% - %B\n", INDENTS[level], e.name, e.data.get_size());
             else
             {
                 const u32 SIZE = 1024 * 6;
                 static char buff[SIZE]; // static to make sure recursive calls dont exhaust task stack
                 u32 count = e.read(buff, SIZE-1);
                 buff[count-1] = '\0';
                 klog.format("%% - %B, %\n", INDENTS[level], e.data.get_size(), e.data.get_size(), buff);
             }
         }
         return true;
     };
    klog.format("%:\n", path);
    traverse_tree(v, directory, 1, on_entry);
}


//void delete_tree_but(VolumeFat32& v, string root, string but) {
//    SimpleDentryFat32 directory;
//    v.get_entry(root, directory);
//
//    auto on_entry = [&](const SimpleDentryFat32& e, u8 level) -> bool {
//        v.delete_entry(e.name);
//        return true;
//    };
//
//    traverse_tree(v, directory, 1, on_entry);
//}

void Fat32Demo::print_hdd_info(AtaDevice& hdd) {
    if (!MassStorageMsDos::verify(hdd)) {
        klog.format("Not MBR formatted device\n");
        return;
    }

    MassStorageMsDos ms(hdd);

    //auto& volumes = ms.get_volumes();
    auto &v = ms.get_volumes()[1];
    //for (auto& v : volumes) {
//        print_volume_info(v);
        for (u16 i = 1; i <= 16; i++) {
              string name = string("/FILE_") +kstd::to_str(i);
              v.delete_entry(name);
        }

        v.delete_entry("/LEVEL1/LEVEL2/LEVEL3/LEVEL3.TXT");
        v.delete_entry("/LEVEL1/LEVEL2/LEVEL3");
        v.delete_entry("/LEVEL1/LEVEL2/LEVEL2.TXT");
        v.delete_entry("/LEVEL1/LEVEL2");
        v.delete_entry("/LEVEL1/LEVEL1.TXT");
        v.delete_entry("/LEVEL1");
        v.delete_entry("/TMP/TMP1.TXT");
        v.delete_entry("/TMP/TMP2.TXT");
        v.delete_entry("/TMP");
        v.move_entry("/TO_BE_~1", "/HAM");
//        print_volume_info(v);



        // create then delete lots of files
//        const u16 NUM_ENTRIES = 256;
//        for (u16 i = 1; i <= NUM_ENTRIES; i++) {
//              string name = string("/F_") +kstd::to_str(i);
//              v.create_entry(name, true);
//        }
//        for (u16 i = 1; i <= NUM_ENTRIES; i++) {
//              string name = string("/F_") +kstd::to_str(i);
//              v.delete_entry(name);
//        }
//
//
//        for (int i = 1; i <= 3; i++) {
//            string dir = "/DIR_" + kstd::to_str(i);
//            v.create_entry(dir, true);
//            for (int j = 1; j <= 5; j++)
//                v.create_entry(dir + "/FILE_" + kstd::to_str(j), false);
//        }


//        v.create_entry("/NUMBERS.TXT", false);
//        auto oda = v.get_entry("/NUMBERS.TXT");
//        string ODA_TEXT;
//        for (u32 i = 0; i < 1024; i++)
//            ODA_TEXT += kstd::to_str(1000 + i) + " ";
//        oda.write(ODA_TEXT.data(), ODA_TEXT.length());
//        print_volume_info(v);
//        print_tree(v, "/");
//        v.delete_entry("/NUMBERS.TXT");
//    }
}

} /* namespace demos */
