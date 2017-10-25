/**
 *   @file: Terminal.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Terminal.h"
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
#include "cmds/specificelfrun.h"
//#include "cmds/cp.h"

#include "syscalls.h"

using namespace ustd;
using namespace cmds;
using namespace middlespace;

namespace terminal {

Terminal::Terminal(u64 arg) {

    u16 vga_width;
    u16 vga_height;
    syscalls::vga_get_width_height(&vga_width, &vga_height);
    printer = new ScrollableScreenPrinter(0, 0, vga_width-1, vga_height-1);
    env.printer = printer;
    user_input.printer = printer;

//    env.klog = &klog;

    // the commands will later be run as elf programs in user space
    install_cmd(new cmds::pwd(&env), "pwd");
//    install_cmd<cmds::log>("klog");
//    install_cmd<cmds::lscpu>("lscpu");
//    install_cmd<cmds::ps>("ps");
//    install_cmd<cmds::free>("free");
//    install_cmd<cmds::lspci>("lspci");
//    install_cmd<cmds::date>("date");
//    install_cmd<cmds::mb2>("mb2");
//    install_cmd<cmds::elfinfo>("elfinfo");
    install_cmd(new cmds::elfrun(&env), "elfrun");
//    install_cmd<cmds::df>("df");
    install_cmd(new cmds::ls(&env), "ls");
//    install_cmd<cmds::cat>("cat");
    install_cmd(new cmds::cd(&env), "cd");
//    install_cmd<cmds::rm>("rm");
//    install_cmd<cmds::mv>("mv");
//    install_cmd<cmds::cp>("cp");
//    install_cmd<cmds::echo>("echo");
//    install_cmd<cmds::mkdir>("mkdir");
//    install_cmd<cmds::tail>("tail");
//    install_cmd<cmds::trunc>("trunc");
//    install_cmd<cmds::test_fat32>("test_fat32");


    // install commands that are in form of separate ELF programs, located under given directory
    install_external_commands("/BIN");
}

/**
 * @brief   Install commands that are exteral ELF files located in filesystem under "dir"
 */
void Terminal::install_external_commands(const string& dir) {
    int fd = syscalls::open(dir.c_str());

    // no such directory
    if (fd < 0)
        return;

    // get "dir" contents
    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    FsEntry* entries = new FsEntry[MAX_ENTRIES];
    int count = syscalls::enumerate(fd, entries, MAX_ENTRIES);

    // install commands that were found
    for (int i = 0; i < count; i++) {
        FsEntry& e = entries[i];

        // dont install directory. Should also check if it is actually an ELF file...
        if (e.is_directory)
            continue;

        string cmd_absolute_path = format("%/%", dir, e.name);
        printer->format("Terminal::installing % from %\n", e.name, cmd_absolute_path);
        install_cmd(new cmds::specificelfrun(&env, cmd_absolute_path), e.name);
    }

    syscalls::close(fd);
    delete[] entries;
}

/**
 * @brief   Install command "cmd" under name "cmd_name"
 */
void Terminal::install_cmd(cmds::CmdBase* cmd, const string& cmd_name) {
    cmd_collection.install(cmd_name, cmd);
    user_input.install(cmd_name);
}

/**
 * @brief   Run the REPL (Read, Eval, Print, Loop)
 */
void Terminal::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        user_input.prompt(env.cwd);

        // get command
        string cmd = get_line();

        // process command
        process_cmd(cmd);
    };
}

bool Terminal::init() {
    keyboard = syscalls::open("/dev/keyboard");
    if (keyboard < 0) {
        return false;
    }

    stdout = syscalls::open("/dev/stdout");
    if (stdout < 0) {
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

    return user_input.get_text();
}

Key Terminal::get_key() {
    const u32 BUFF_SIZE = 512;
    char buff[BUFF_SIZE];

    // wait until last_key is set by keyboard interrupt. Keys might be missed if stroking faster than Terminal gets CPU time :)
    Key key;
    while (!(syscalls::read(keyboard, &key, sizeof(key)) > 0)) {

        // while waiting for key to arrive, check if there is something in stdout to be printed out
        u32 count;
        while ((count = syscalls::read(stdout, buff, BUFF_SIZE - 1)) > 0) {
            buff[count] = '\0';
            printer->format("%", buff);
        }
        syscalls::usleep(0);
    }

    return key;
}

void Terminal::process_key(Key key) {
    // functional key
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up: {
            user_input.suggest_cmd(cmd_history.get_prev());
            break;
        }

        case Key::Down: {
            user_input.suggest_cmd(cmd_history.get_next());
            break;
        }

        case Key::Tab: {
            user_input.help_me_out(env.cwd);
            break;
        }

        case Key::Enter: {
            printer->format('\n');
            cmd_history.set_to_latest();
            break;
        }

        case Key::Backspace: {
            user_input.backspace();
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
    // just an ASCII character
    else {
        user_input.putc(key);
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

    user_input.clear();
}

} /* namespace terminal */
