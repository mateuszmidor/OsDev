/**
 *   @file: LineBuffer.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_LINEBUFFER_H_
#define SRC_UTILS_LINEBUFFER_H_

#include "types.h"
#include "ustd.h"

namespace terminal {

class LineBuffer {
public:
    LineBuffer();
    u32 count() const;
    void push_back(const ustd::string& line);
    void putc(char c);
    void backspace();
    void newline();
    const ustd::string& back() const;
    const ustd::string& operator[](u32 index) const;

private:
    ustd::vector<ustd::string> lines;
};
} /* namespace terminal */

#endif /* SRC_UTILS_LINEBUFFER_H_ */
