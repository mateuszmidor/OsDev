/**
 *   @file: smem.cpp
 *
 *   @date: Oct 16, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"

unsigned char buffer[1024*1024*8]; // 8MB

/**
 * @brief   Entry point
 * @return  0 if buffer properly initialized to 0, first nonzero entry value otherwise
 */
int main(int argc, char* argv[]) {
    // Check if global variable properly initialized to 0
    for (unsigned int i = 0; i < sizeof(buffer); i++)
        if (buffer[i] != 0)
            return buffer[i];

    return 0;
}




