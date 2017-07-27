/*
 * free.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: mateusz
 */

#include "free.h"

extern size_t bump_addr;
namespace cmds {

void free::run() {
    env->printer->format("Used memory so far: % KB\n", (bump_addr - 2 * 1024 * 1024) / 1024);
}
} /* namespace cmds */
