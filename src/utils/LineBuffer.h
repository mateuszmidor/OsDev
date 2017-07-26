/**
 *   @file: LineBuffer.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_LINEBUFFER_H_
#define SRC_UTILS_LINEBUFFER_H_

#include "types.h"
#include "kstd.h"

namespace utils {

class LineBuffer {
public:
    LineBuffer();
    u32 count() const;
    void push_back(const kstd::string& line);
    void putc(char c);
    void backspace();
    void newline();
    const kstd::string& back() const;
    const kstd::string& operator[](u32 index) const;

private:
    kstd::vector<kstd::string> lines;
};
} /* namespace utils */

#endif /* SRC_UTILS_LINEBUFFER_H_ */
