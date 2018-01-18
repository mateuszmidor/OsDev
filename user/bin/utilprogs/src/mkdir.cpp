/**
 *   @file: mkdir.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

const char ERROR_NO_INPUT_FILE[]    = "mkdir: please provide directory path\n";
const char ERROR_CANT_CREATE[]      = "mkdir: cant create given directory\n";

using namespace cstd::ustd;

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print(ERROR_NO_INPUT_FILE);
        return 1;
    }

    const char* path = argv[1];

    int result = syscalls::mkdir(path);
    if (result < 0) {
        cout::print(ERROR_CANT_CREATE);
        return 1;
    }

    return 0;
}
