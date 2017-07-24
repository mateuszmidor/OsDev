/*
 * Fat32Demo.h
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_FAT32DEMO_H_
#define SRC__DEMOS_FAT32DEMO_H_

#include <memory>
#include "KernelLog.h"
#include "VolumeFat32.h"
#include "kstd.h"

namespace demos {

class Fat32Demo {
public:
    Fat32Demo();
    void run();

private:
    using OnTreeEntryFound = std::function<bool(const filesystem::SimpleDentryFat32&, u8)>;

    void print_volume_info(filesystem::VolumeFat32& v);
    void print_file(filesystem::VolumeFat32& v, kstd::string filename);
    void print_tree(filesystem::VolumeFat32& v, kstd::string path);
    void print_hdd_info(drivers::AtaDevice& hdd);
    void traverse_tree(filesystem::VolumeFat32& v, const filesystem::SimpleDentryFat32& entry, u8 level, const OnTreeEntryFound& user_on_entry);

    utils::KernelLog& klog;
};

} /* namespace demos */

#endif /* SRC__DEMOS_FAT32DEMO_H_ */
