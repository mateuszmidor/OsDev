/**
 *   @file: mv.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

const char ERROR_NO_INPUT[]     = "mv: please provide source and destination names\n";
const char ERROR_CANT_MOVE[]    = "mv: cant move given entities\n";

using namespace cstd::ustd;

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout::print(ERROR_NO_INPUT);
        return 1;
    }

    const char* old_path = argv[1];
    const char* new_path = argv[2];

    int result = syscalls::rename(old_path, new_path);
    if (result < 0) {
        cout::print(ERROR_CANT_MOVE);
        return 1;
    }

    return 0;
}
