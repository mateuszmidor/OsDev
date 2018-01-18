/**
 *   @file: LineBuffer.cpp
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#include "LineBuffer.h"

using namespace cstd;

namespace terminal {

LineBuffer::LineBuffer() {
    // add first, empty line to the buffer
    newline();
}

u32 LineBuffer::count() const {
    return lines.size();
}

void LineBuffer::clear() {
    lines.clear();
    newline();
}

void LineBuffer::push_back(const string& line) {
    lines.push_back(line);
}

void LineBuffer::putc(char c) {
    lines.back().push_back(c);
}

void LineBuffer::backspace() {
    if (lines.back().empty() && (lines.size() > 1)) // if the bottom line is empty and it is not the only one line in the buffer
        lines.pop_back();                           // remove the line

    if (!lines.back().empty())      // if the bottom line is not empty
        lines.back().pop_back();    // remove last character from the line
}

void LineBuffer::newline() {
    lines.push_back("");
}

const string& LineBuffer::back() const {
    return lines.back();
}

const string& LineBuffer::operator[](u32 index) const {
    return lines[index];
}

} /* namespace terminal */
