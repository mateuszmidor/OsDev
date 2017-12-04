/**
 *   @file: Terminal.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Terminal.h"
#include "StringUtils.h"
#include "Cin.h"

#include "cmds/cd.h"
#include "cmds/elfrun.h"
#include "cmds/specificelfrun.h"

#include "syscalls.h"

using namespace ustd;
using namespace cmds;
using namespace middlespace;

namespace terminal {

Terminal::Terminal(u64 arg) : user_input(printer) {
    // start at /HOME
    syscalls::chdir("/HOME");

    // setup screen printer
    u16 vga_width;
    u16 vga_height;
    syscalls::vga_get_width_height(&vga_width, &vga_height);
    printer.reset(std::make_shared<ScrollableScreenPrinter>(0, 0, vga_width-1, vga_height-1));

    // install internal commands
    install_cmd(new cmds::elfrun, "elfrun");
    install_cmd(new cmds::cd,"cd");

    // install external commands that are in form of separate ELF programs, located under given directory
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

        string cmd_absolute_path = StringUtils::format("%/%", dir, e.name);
        string lowercase_name = StringUtils::to_lower_case(e.name);
        printer.get()->format("Terminal::installing % from %\n", lowercase_name, cmd_absolute_path);
        install_cmd(new cmds::specificelfrun(cmd_absolute_path), lowercase_name);
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
        user_input.prompt();

        // get command
        string cmd = cin::readln();

        // process command
        process_cmd(cmd);
    };
}

bool Terminal::init() {
    fd_keyboard = syscalls::open("/dev/keyboard");
    if (fd_keyboard < 0) {
        return false;
    }

    fd_stdin = syscalls::open("/dev/stdin");
    if (fd_stdin < 0) {
        return false;
    }

    fd_stdout = syscalls::open("/dev/stdout");
    if (fd_stdout < 0) {
        return false;
    }

    printer.get()->clear_screen();

    // start a thread reading /dev/keyboard
    syscalls::task_lightweight_run((unsigned long long)key_processor_thread, (unsigned long long)this, "terminal_key_processor");

    // start a thread printing /dev/stdout to the screen
    syscalls::task_lightweight_run((unsigned long long)stdout_printer_thread, (unsigned long long)this, "terminal_stdout_printer");

    return true;
}


void Terminal::on_key_down(Key key) {
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
            user_input.help_me_out();
            break;
        }

        case Key::Backspace: {
            user_input.backspace();
            break;
        }

        case Key::PgUp: {
            printer.get()->scroll_up(4);
            break;
        }

        case Key::PgDown: {
            printer.get()->scroll_down(4);
            break;
        }

        case Key::Home: {
            printer.get()->scroll_to_begin();
            break;
        }

        case Key::End: {
            printer.get()->scroll_to_end();
            break;
        }

        case Key::Esc:
            break;

        case Key::Enter: {
            user_input.putc('\n');
            syscalls::write(fd_stdin, user_input.get_text().c_str(), user_input.get_text().length()); // this makes cin::readln() work :)
            user_input.clear();
            break;
        }

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

    auto cmd_args = ustd::StringUtils::split_string(cmd, ' ');
    bool run_in_background = false;
    if (cmd_args.back() == "&") {
        run_in_background = true;
        cmd_args.pop_back();
    }

    if (CmdBase* task = cmd_collection.get(cmd_args[0])) {
        task->run(cmd_args, run_in_background);
        cmd_history.append(cmd);
        cmd_history.set_to_latest();
    }
    else
        printer.get()->format("Unknown command: '%'\n", cmd);
}

/**
 * @brief   This function reads /dev/stdout in a loop and print it to the screen
 * @note    This function is run in a separate thread
 */
void Terminal::stdout_printer_thread(Terminal* term) {
    const u32 BUFF_SIZE = 512;
    char buff[BUFF_SIZE];

    while (true) {
        ssize_t count = syscalls::read(term->fd_stdout, buff, BUFF_SIZE - 1);
        buff[count] = '\0';
        term->printer.get()->format("%", buff);
    }
}

/**
 * @brief   This function reads /dev/keyboard and runs Terminal::on_key_down
 * @note    This function is run in a separate thread
 */
void Terminal::key_processor_thread(Terminal* term) {
    Key key;

    while (true) {
        syscalls::read(term->fd_keyboard, &key, sizeof(key));
        term->on_key_down(key);
    }
}

} /* namespace terminal */
