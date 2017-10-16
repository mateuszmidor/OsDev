/**
 *   @file: dmem.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"

/**
 * @brief   Entry point
 * @return  0 if buffer properly initialized to 0, first nonzero entry otherwise
 */
int main(int argc, char* argv[]) {
    char* buff = new char[1024 * 1024 * 8];

    buff[0] = 'A';
    buff[sizeof(buff) - 1] = 'Z';

    return 0;
}

