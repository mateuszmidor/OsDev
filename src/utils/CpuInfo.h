/**
 *   @file: CpuInfo.h
 *
 *   @date: Jun 7, 2017
 * @author: mateusz
 */


#ifndef SRC_CPUINFO_H_
#define SRC_CPUINFO_H_

namespace utils {

class CpuInfo {
public:
    CpuInfo();
    void print_to_klog() const;

private:
    char vendor[13];
    void getCpuVendor(char buff[13]);
};

} // namespace utils
#endif /* SRC_CPUINFO_H_ */
