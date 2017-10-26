/**
 *   @file: cp.cpp
 *
 *   @date: Oct 25, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "ustd.h"
#include "utils.h"

using namespace ustd;


const char ERROR_NO_INPUT_PATHS[] = "cp: please specify src and dst paths\n";
const char ERROR_INPUT_NO_EXISTS[] = "cp: source file does not exist\n";
const char ERROR_SOURCE_IS_DIR[] = "cp: source is a directory. Recursive copy not implemented yet\n";
const char ERROR_CANT_CREATE_DESTIN[] = "cp: cant create destination file\n";
const char MSG_DONE[] = "cp: copying done.\n";

const u32 BUFF_SIZE = 1024;
char buff[BUFF_SIZE];


bool exists(const char filename[]) {
    struct stat s;
    return syscalls::stat(filename, &s) != -1;
}

bool is_directory(const char filename[]) {
    struct stat s;
    if (syscalls::stat(filename, &s) == -1) {
        return false;
    }

    return (s.st_mode == S_IFDIR);
}

string extract_file_name(const string& path) {
    size_t pivot = path.rfind('/');
    return path.substr(pivot+1, path.size());
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print(ERROR_NO_INPUT_PATHS);
        return -1;
    }

    const char* src_path = argv[1];
    const char* dst_path = argv[2];


    // source must exist
    int src_fd = syscalls::open(src_path);
    if (src_fd == -1) {
        print(ERROR_INPUT_NO_EXISTS);
        return -1;
    }

    // copying directory should use recursive approach
    if (is_directory(src_path)) {
        print(ERROR_SOURCE_IS_DIR);
        return -1;   // TODO: implement recursive copy
    }

    // check if dst describes destination directory or full destination path including filename
    string final_dst_filename;
    if (exists(dst_path) && is_directory(dst_path))
        final_dst_filename = string(dst_path) + "/" + extract_file_name(src_path);
    else
        final_dst_filename = dst_path;

    // create dst file
    int dst_fd = syscalls::creat(final_dst_filename.c_str());
    if (dst_fd < 0) {
        print(ERROR_CANT_CREATE_DESTIN);
        return -1;
    }

    // dest created, just copy contents
    ssize_t count;
    while ((count = syscalls::read(src_fd, buff, BUFF_SIZE)) > 0) {
        syscalls::write(dst_fd, buff, count);
    }

    syscalls::close(src_fd);
    syscalls::close(dst_fd);
    print(MSG_DONE);
    return 0;
}
