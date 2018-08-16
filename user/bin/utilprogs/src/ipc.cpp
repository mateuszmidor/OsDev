/**
 *   @file: ipc.cpp
 *
 *   @date: Aug 16, 2018
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "String.h"
#include "Vector.h"

using namespace cstd;
using namespace cstd::ustd;

const char FIFO_NAME[] {"fifo"};

s64 read_fifo_thread() {
    auto fd = syscalls::open(FIFO_NAME);
    if (fd < 0)
        return 1;

    char buff[128];
    do {
        auto count = syscalls::read(fd, buff, sizeof(buff));
        cout::print(buff, count);
    } while (buff[0] != '#');

    syscalls::close(fd);
    return 0;
}

s64 write_fifo_thread(unsigned ret) {
    auto fd = syscalls::open(FIFO_NAME);
    if (fd < 0)
        return 1;

    vector<string> strings {
        "Litwo!\n",
        "Ojczyzno moja!\n",
        "Ty jestes jak zdrowie!\n",
        "Ile cie trzeba cenic\n",
        "Ten tylko sie dowie\n",
        "Kto Cie stracil.\n",
        "#"
    };

    for (const auto& s : strings) {
        syscalls::write(fd, s.c_str(), s.length());
        syscalls::msleep(1000);
    }

    syscalls::close(fd);
    return 0;
}

s64 run_thread(unsigned long long func, unsigned arg, const char name[]) {
    auto tid = syscalls::task_lightweight_run(func, arg, name);
    switch (tid) {
    case -ENOMEM:
        cout::print("ipc: no memory to run thread. Exit.\n");
        break;

    case -EPERM:
        cout::print("ipc: too many threads running aleady. Exit.\n");
        break;

    default:
        break;
    }
    return tid;
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    auto result = syscalls::mknod(FIFO_NAME, S_IFIFO);
    if (result < 0)
        return 1;


    auto t1 = run_thread((unsigned long long)read_fifo_thread, 0, "read_fifo_thread");
    if (t1 < 0)
        return 1;

    auto t2 = run_thread((unsigned long long)write_fifo_thread, 0, "write_fifo_thread");
    if (t2 < 0)
        return 1;

    syscalls::task_wait(t1);
    syscalls::task_wait(t2);
    cout::print("\nIpc done.\n");

    syscalls::unlink(FIFO_NAME);
    return 0;
}
