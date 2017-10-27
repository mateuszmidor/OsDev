/**
 *   @file: dmem.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"
#include <string>

/**
 * @brief   Entry point
 * @return  0 no matter what
 */
const char msg1[] = "dmem: this is a pretty long message";
const char msg2[] = "coming from dynamic memory\n";
int main(int argc, char* argv[]) {
    std::string a = msg1;
    std::string b = msg2;
    std::string concat = a + ", " + b;
    print(concat.c_str());
    return 0;
}

