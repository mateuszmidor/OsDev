/**
 *   @file: tree.cpp
 *
 *   @date: Dec 14, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "StringUtils.h"
#include "Vector.h"
#include <bitset>

const char ERROR_PATH_NOT_EXISTS[]  = "tree: path doesnt exist\n";
const char ERROR_OPENING_DIR[]      = "tree: cant open specified directory\n";
const char ERROR_NOT_DIRECTORY[]    = "tree: path is not directory\n";
char buff[256];
std::bitset<64> current_row_levels;

using namespace ustd;
using namespace middlespace;


/**
 * @brief   Make a single row of a tree like below (filename is added later, at the end of the row)

[/HOME]
  |---file1.txt
  |---file2.txt
  |---[DOCS]
  |     `---cv.pdf      // eg here the resulting row will be "  |     `---"
  `---[PICS]
 */
string make_tree_row_for_element(u32 level, bool last_element) {
    constexpr char VERTICAL         {'\xB3'};   // |
    constexpr char HORIZONTAL       {'\xC4'};   // -
    constexpr char LASTONE          {'\xC0'};   // |_
    constexpr u32 ELEMENT_OFFSET    {4};        // element offset from tree branch
    constexpr u32 SUBTREE_OFFSET    {2};        // subtree offset from parent branch

    if (level == 0)
        return {};

    string row(SUBTREE_OFFSET, ' ');
    if (last_element)
        row += string(1, LASTONE) + string(ELEMENT_OFFSET-1, HORIZONTAL);    // |______
    else
        row += string(1, VERTICAL) + string(ELEMENT_OFFSET-1, HORIZONTAL);   // |------

    // build tree row eg "|    |----file.txt"
    for (s32 i = level - 2; i >=0; i--)
        if (current_row_levels.test(i))
            row = string(SUBTREE_OFFSET, ' ') + VERTICAL + string(ELEMENT_OFFSET - 1, ' ') + row;
        else
            row = string(SUBTREE_OFFSET, ' ') + string(ELEMENT_OFFSET, ' ') + row;

    return row;
}

string format_size(u32 size) {
    if (size > 1024*1024*1024)
        return StringUtils::format("%G", size / 1024 / 1024 / 1024);
    else if (size > 1024*1024)
        return StringUtils::format("%M", size / 1024 / 1024);
    else if (size > 1024)
        return StringUtils::format("%K", size / 1024);
    else
        return StringUtils::format("%", size);
}

void print_file(const char name[], u32 size, u32 level, bool last_one) {
    string ind = make_tree_row_for_element(level, last_one);
    string siz = format_size(size);
    cout::format("%% - %\n", ind, name, siz);
}

void print_dir(const char name[], u32 level, bool last_one) {
    string ind = make_tree_row_for_element(level, last_one);
    cout::format("%[%]\n", ind, name);
}

void traverse_dir(const string& path, u32 level) {
    // open directory
    int fd = syscalls::open(path.c_str());
    if (fd < 0) {
        cout::print(ERROR_OPENING_DIR);
        return;
    }

    // enumerate contents
    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    vector<VfsEntry> entries(MAX_ENTRIES);
    auto count = syscalls::enumerate(fd, entries.data(), entries.size());

    // close directory
    syscalls::close(fd);

    // print/traverse contents
    current_row_levels.set(level, true);
    for (auto i = 0; i < count; i++) {
        const auto& e = entries[i];
        bool last_element = (i == count - 1);
        if (last_element)
            current_row_levels.set(level, false);

        if (e.is_directory) {
            string str_name {e.name};
            if (str_name == "." || str_name == "..")
                continue;

            print_dir(e.name, level+1, last_element);

            string sub_path = path + "/" + str_name;
            traverse_dir(sub_path, level+1);
        }
        else
            print_file(e.name, e.size, level+1, last_element);
    }
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

    // given path is file - bad
    if (s.st_mode == S_IFREG) {
        cout::print(ERROR_NOT_DIRECTORY);
        return 1;
    }

    // given path is directory - good
    print_dir(path, 0, false);
    traverse_dir(path, 0);

    return 0;
}
