/**
 *   @file: Terminal.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Terminal.h"
#include "ScrollableScreenPrinter.h"
//#include "KernelLog.h"
//#include "TaskManager.h"
//#include "CpuInfo.h"

//#include "cmds/df.h"
#include "cmds/pwd.h"
//#include "cmds/log.h"
//#include "cmds/lscpu.h"
#include "cmds/ls.h"
//#include "cmds/cat.h"
//#include "cmds/ps.h"
//#include "cmds/free.h"
#include "cmds/cd.h"
//#include "cmds/rm.h"
//#include "cmds/mv.h"
//#include "cmds/echo.h"
//#include "cmds/mkdir.h"
//#include "cmds/tail.h"
//#include "cmds/trunc.h"
//#include "cmds/testfat32.h"
//#include "cmds/lspci.h"
//#include "cmds/date.h"
//#include "cmds/mb2.h"
//#include "cmds/elfinfo.h"
#include "cmds/elfrun.h"
//#include "cmds/cp.h"

#include "syscalls.h"
#include <algorithm>

using namespace ustd;
using namespace utils;
using namespace middlespace;
using namespace cmds;
//using namespace multitasking;

namespace terminal {

const string Terminal::PROMPT {"> "};

Terminal::Terminal(u64 arg) {

    u16 vga_width;
    u16 vga_height;
    syscalls::vga_get_width_height(&vga_width, &vga_height);
    printer = new ScrollableScreenPrinter(0, 0, vga_width-1, vga_height-1);
    env.printer = printer;
//    env.klog = &klog;

    // the commands will later be run as elf programs in user space
    install_cmd<cmds::pwd>("pwd");
//    install_cmd<cmds::log>("klog");
//    install_cmd<cmds::lscpu>("lscpu");
//    install_cmd<cmds::ps>("ps");
//    install_cmd<cmds::free>("free");
//    install_cmd<cmds::lspci>("lspci");
//    install_cmd<cmds::date>("date");
//    install_cmd<cmds::mb2>("mb2");
//    install_cmd<cmds::elfinfo>("elfinfo");
    install_cmd<cmds::elfrun>("elfrun");
//    install_cmd<cmds::df>("df");
    install_cmd<cmds::ls>("ls");
//    install_cmd<cmds::cat>("cat");
    install_cmd<cmds::cd>("cd");
//    install_cmd<cmds::rm>("rm");
//    install_cmd<cmds::mv>("mv");
//    install_cmd<cmds::cp>("cp");
//    install_cmd<cmds::echo>("echo");
//    install_cmd<cmds::mkdir>("mkdir");
//    install_cmd<cmds::tail>("tail");
//    install_cmd<cmds::trunc>("trunc");
//    install_cmd<cmds::test_fat32>("test_fat32");
}

void Terminal::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        printer->format("% %", env.cwd, PROMPT);

        // get command
        string cmd = get_line();

        // process command
        process_cmd(cmd);
    };
}

bool Terminal::init() {
//    auto& klog = KernelLog::instance();

    keyboard = syscalls::open("/dev/keyboard");
    if (keyboard < 0) {
//        klog.format("Terminal::init: no /dev/keyboard \n");
        return false;
    }

    stdout = syscalls::open("/dev/stdout");
    if (stdout < 0) {
//        klog.format("Terminal::init: no /dev/stdout \n");
        return false;
    }

    printer->clear_screen();
    return true;
}

string Terminal::get_line() {
    Key key;
    do  {
        key = get_key();
        process_key(key);
    } while (key != Key::Enter);

    return cl_user_input;
}

Key Terminal::get_key() {
    u32 BUFF_SIZE = 512;
    char buff[BUFF_SIZE];

    // wait until last_key is set by keyboard interrupt. Keys might be missed if stroking faster than Terminal gets CPU time :)
    while (!(syscalls::read(keyboard, &last_key, sizeof(last_key)) > 0)) {
        u32 count;
        while ((count = syscalls::read(stdout, buff, BUFF_SIZE - 1)) > 0) {
            buff[count] = '\0';
            printer->format("%", buff);
        }
        syscalls::usleep(0);
    }

    Key key = last_key;
    last_key = Key::INVALID;
    return key;
}

/**
 * @brief   Erase entire command line input and replace with "cmd"
 */
void Terminal::suggest_cmd(const string& cmd) {
    for (s16 i = cl_user_input.length()-1; i >= 0 ; i--)
        printer->format('\x08');    // backspace

    printer->format(cmd);
    cl_user_input =  cmd;
}

/**
 * @brief   Erase last segment of command line parameter and replace with "param"
 */
void Terminal::suggest_param(const string& param) {
    for (s16 i = cl_user_input.length()-1; i >= 0 ; i--) {
        if (cl_user_input[i] == ' ')    // segment break (beginning of parameter)
            break;

        if (cl_user_input[i] == '/')    // path break (beginnig of path segment)
            break;

        printer->format('\x08');    // backspace
        cl_user_input.pop_back();
    }

    printer->format(param);
    cl_user_input += param;
}

void Terminal::process_key(Key key) {
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up: {
            suggest_cmd(cmd_history.get_prev());
            break;
        }

        case Key::Down: {
            suggest_cmd(cmd_history.get_next());
            break;
        }

        case Key::Tab: {
            bool multiple_results;
            string filter_result;


            if (cl_user_input.find(' ') == string::npos) { // command not given, suggest command
                std::tie(multiple_results, filter_result) = cmd_collection.filter(cl_user_input);
                if (multiple_results)
                    printer->format("\n  % \n% %%", filter_result, env.cwd, PROMPT, cl_user_input);
                else
                    suggest_cmd(filter_result);
            }
            else { // suggest param
                string second_segment = cl_user_input.substr(cl_user_input.rfind(' ') + 1, cl_user_input.length());
                string common_pattern;
                std::tie(multiple_results, filter_result, common_pattern) = cd_filter(second_segment);
                if (multiple_results)
                    printer->format("\n  % \n% %%", filter_result, env.cwd, PROMPT, cl_user_input);
              //  else
                if (!common_pattern.empty())
                    suggest_param(common_pattern);
            }


            break;
        }

        case Key::Enter: {
            printer->format('\n');
            cmd_history.set_to_latest();
            break;
        }

        case Key::Backspace: {
            if (!cl_user_input.empty()) {
                cl_user_input.pop_back();
                printer->format('\x08');    // backspace
            }
            break;
        }

        case Key::PgUp: {
            printer->scroll_up(4);
            break;
        }

        case Key::PgDown: {
            printer->scroll_down(4);
            break;
        }

        case Key::Home: {
            printer->scroll_to_begin();
            break;
        }

        case Key::End: {
            printer->scroll_to_end();
            break;
        }

        case Key::Esc:
            break;

        default:;
        }
    }
    else {
        printer->format(key);
        cl_user_input.push_back(key);
    }
}

void Terminal::process_cmd(const string& cmd) {
    if (cmd.empty())
        return;

    auto cmd_args = ustd::split_string<vector<string>>(cmd, ' ');
    if (CmdBase* task = cmd_collection.get(cmd_args[0])) {
        env.cmd_args = cmd_args;
        task->run();
        cmd_history.append(cmd);
    }
    else
        printer->format("Unknown command: %\n", cmd);

    cl_user_input.clear();
}

std::tuple<bool, string, string> Terminal::cd_filter(const string& param) {
    string path;
    string pattern;

    size_t pivot = param.rfind('/');
    if (pivot != string::npos) {
        path = param.substr(0, pivot+1);
        if (path[0] != '/')
            path = env.cwd + "/" + path;

        pattern = param.substr(pivot+1, param.length());
    }
    else {
        path = env.cwd;
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

void Terminal::print_klog() {
//    auto& klog = KernelLog::instance();
//    printer->format("%\n", klog.get_text());
}


} /* namespace terminal */
