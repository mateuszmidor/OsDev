/**
 *   @file: ls.cpp
 *
 *   @date: Oct 26, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "StringUtils.h"

const char ERROR_PATH_NOT_EXISTS[]  = "ls: path doesnt exist\n";
const char ERROR_OPENING_DIR[]      = "ls: cant open specified directory\n";
char buff[256];

using namespace ustd;
using namespace middlespace;

void print_file(const char name[], u32 size) {
    cout::format("% - % bytes\n", name, size);
}

void print_dir(const char name[]) {
    cout::format("[%]\n", name);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    char* path;
    if (argc == 1) {
        syscalls::getcwd(buff, sizeof(buff) - 1);
        path = buff;
    }
    else
        path = argv[1];


    // check exists
    struct stat s;
    if (syscalls::stat(path, &s) < 0) {
        cout::print(ERROR_PATH_NOT_EXISTS);
        return 1;
    }

    // ls file
    if (s.st_mode == S_IFREG) {
        print_file(path, s.st_size);
        return 0;
    }

    // ls directory
    int fd = syscalls::open(path);
    if (fd < 0) {
        cout::print(ERROR_OPENING_DIR);
        return 1;
    }

    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    VfsEntry* entries = new VfsEntry[MAX_ENTRIES];

    int count = syscalls::enumerate(fd, entries, MAX_ENTRIES);
    for (int i = 0; i < count; i++)
        if (entries[i].is_directory)
            print_dir(entries[i].name);
        else
            print_file(entries[i].name, entries[i].size);

    delete[] entries;
    syscalls::close(fd);

    return 0;
}
