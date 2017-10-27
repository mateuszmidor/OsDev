/**
 *   @file: mv.cpp
 *
 *   @date: Oct 27, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"

const char ERROR_NO_INPUT[] = "Please provide source and destination names";
const char ERROR_CANT_MOVE[] = "Cant move given entities\n";


/**
 * @brief   Entry point
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        print(ERROR_NO_INPUT);
        return -1;
    }

    const char* old_path = argv[1];
    const char* new_path = argv[2];

    int result = syscalls::rename(old_path, new_path);
    if (result < 0) {
        print(ERROR_CANT_MOVE);
        return -1;
    }

    return 0;
}
