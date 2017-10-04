/**
 *   @file: kmain.cpp
 *
 *   @date: Oct 4, 2017
 * @author: Mateusz Midor
 */

/**
 * @brief   Just print hello msg showing that higher half kernel works
 */
extern "C" void kmain() {
    char* hello = "Hello in higher half!";
    char *buff = (char*)0xFFFFFFFF800B8140;

    while (*hello) {
        buff[0] = *hello;
        buff[1] = 0xf4;
        buff += 2;
        hello++;
    }

//    buff = (char*)0xFFFFFFFa800B8140;
//    buff[0] = 1;
}


