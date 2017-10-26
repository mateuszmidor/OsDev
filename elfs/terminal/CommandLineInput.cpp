/**
 *   @file: CommandLineInput.cpp
 *
 *   @date: Oct 24, 2017
 * @author: Mateusz Midor
 */

#include "CommandLineInput.h"
#include "syscalls.h"

using namespace ustd;
using namespace middlespace;
namespace terminal {

const char CommandLineInput::PROMPT[] {"> "};
char CommandLineInput::cwd[256];

CommandLineInput::CommandLineInput() : printer(nullptr) {
}

void CommandLineInput::prompt() {
    char cwd[256];
    if (syscalls::getcwd(cwd, 255) < 0)
        printer->format("[UNKNOWN CWD] %", PROMPT);
    else
        printer->format("% %", cwd, PROMPT);
}

void CommandLineInput::backspace() {
    if (!input.empty()) {
        input.pop_back();
        printer->format('\x08');    // backspace
    }
}

void CommandLineInput::putc(char c) {
    input.push_back(c);
    printer->format(c);
}

void CommandLineInput::clear() {
    input.clear();
}

void CommandLineInput::install(const ustd::string& cmd_name) {
    known_commands.push_back(cmd_name);
}

void CommandLineInput::help_me_out() {
    if (!syscalls::getcwd(cwd, sizeof(cwd)))
        return;

    bool multiple_results;
    string filter_result;


    if (input.find(' ') == string::npos) { // command not given, suggest command
        std::tie(multiple_results, filter_result) = command_filter(input);
        if (multiple_results)
            printer->format("\n  % \n% %%", filter_result, cwd, PROMPT, input);
        else
            suggest_cmd(filter_result);
    }
    else { // suggest param
        string second_segment = input.substr(input.rfind(' ') + 1, input.length());
        string common_pattern;
        std::tie(multiple_results, filter_result, common_pattern) = folder_filter(cwd, second_segment);
        if (multiple_results)
            printer->format("\n  % \n% %%", filter_result, cwd, PROMPT, input);
      //  else
        if (!common_pattern.empty())
            suggest_param(common_pattern);
    }
}
/**
 * @brief   Erase entire command line input and replace with "cmd"
 */
void CommandLineInput::suggest_cmd(const string& cmd) {
    for (s16 i = input.length()-1; i >= 0 ; i--)
        printer->format('\x08');    // backspace

    printer->format(cmd);
    input =  cmd;
}

/**
 * @brief   Erase last segment of command line parameter and replace with "param"
 */
void CommandLineInput::suggest_param(const string& param) {
    for (s16 i = input.length()-1; i >= 0 ; i--) {
        if (input[i] == ' ')    // segment break (beginning of parameter)
            break;

        if (input[i] == '/')    // path break (beginnig of path segment)
            break;

        printer->format('\x08');    // backspace
        input.pop_back();
    }

    printer->format(param);
    input += param;
}

/**
 * @brief   Match known commands against name patter
 * @param   pattern Command name beginning
 * @return  {false, name} if single command found
 *          {true, name_list} if multiple commands found
 */
std::tuple<bool, string> CommandLineInput::command_filter(const string& pattern) {
    ustd::vector<string> found;
    for (const string& c : known_commands)
        if (c.find(pattern) == 0)
            found.push_back(c);

    if (found.empty())
        return std::make_tuple(false, pattern);
    else if (found.size() == 1)
        return std::make_tuple(false, found.back());
    else
        return std::make_tuple(true, ustd::join_string(" ", found));
}

std::tuple<bool, string, string> CommandLineInput::folder_filter(const string& cwd, const string& param) {
    string path;
    string pattern;

    size_t pivot = param.rfind('/');
    if (pivot != string::npos) {
        path = param.substr(0, pivot+1);
        if (path[0] != '/')
            path = cwd + "/" + path;

        pattern = param.substr(pivot+1, param.length());
    }
    else {
        path = cwd;
        pattern = param;
    }

    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    FsEntry* entries = new FsEntry[MAX_ENTRIES];

    int fd = syscalls::open(path.c_str());
    if (fd < 0) {
        return {};
    }

    ustd::vector<string> found;

    // filter results
    int count = syscalls::enumerate(fd, entries, MAX_ENTRIES);
    for (int i = 0; i < count; i++) {
        const string& name (entries[i].name);
        if (name == "." || name == "..")
            continue;

        if (name.find(pattern) == 0) {
            if (entries[i].is_directory)
                found.push_back(name + "/");
            else
                found.push_back(name);
        }
    }

    // find common pattern
    string common_pattern;
    size_t i = 0;
    bool done = found.empty();
    while (!done) {
        char common = found.front()[i];
        for (const auto& s : found) {
            if (s.length() <= i) {
                done = true;
                break;
            }

            if (s[i] != common) {
                done = true;
                break;
            }
        }
        i++;
    }
    if (!found.empty())
        common_pattern = found.front().substr(0, i-1);

    delete[] entries;
    syscalls::close(fd);

    if (found.empty())
        return std::make_tuple(false, pattern, common_pattern);
    else if (found.size() == 1)
        return std::make_tuple(false, found.back(), common_pattern);
    else
        return std::make_tuple(true, ustd::join_string(" ", found), common_pattern);

}
} /* namespace terminal */
