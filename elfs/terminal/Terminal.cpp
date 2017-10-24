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


/*
    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    FsEntry* entries = new FsEntry[MAX_ENTRIES];

    int fd = syscalls::open("/BIN");
    if (fd < 0)
        return;


    ustd::vector<string> found;

    // filter results
    int count = syscalls::enumerate(fd, entries, MAX_ENTRIES);
    for (int i = 0; i < count; i++)
        if (entries[i].is_directory == false)
            install(entries[i].name);

    syscalls::close(fd);
    */
}

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
