/**
 *   @file: LineBuffer.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_LINEBUFFER_H_
#define SRC_UTILS_LINEBUFFER_H_

#include "types.h"
#include "String.h"
#include "Vector.h"

namespace terminal {

class LineBuffer {
public:
    LineBuffer();
    u32 count() const;
    void clear();
    void push_back(const cstd::string& line);
    void putc(char c);
    void backspace();
    void newline();
    const cstd::string& back() const;
    const cstd::string& operator[](u32 index) const;

private:
    cstd::vector<cstd::string> lines;
};
} /* namespace terminal */

#endif /* SRC_UTILS_LINEBUFFER_H_ */
