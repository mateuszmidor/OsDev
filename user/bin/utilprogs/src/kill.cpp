/*
 * kill.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: Mateusz Midor
 */

#include <algorithm>
#include "_start.h"
#include "syscalls.h"
#include "Cout.h"

using namespace cstd;
using namespace cstd::ustd;


bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

bool is_int(const char num[]) {
	auto len = strlen(num);
	return std::all_of(num, num+len, is_digit);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print("kill: please speficy task id for kill\n");
        return 1;
    }

    if (!is_int(argv[1])) {
        cout::print("kill: invalid task id. Please specify a number\n");
        return 1;
    }

    auto task_id = StringUtils::to_int(argv[1]);
    auto kill_result = syscalls::kill(task_id, SIGKILL);
    if (kill_result < 0) {
        cout::format("kill: could not kill task: %; error code: %\n", task_id, -kill_result);
        return 1;
    }

    cout::print("kill: SIGKILL sent\n");
    return 0;
}



