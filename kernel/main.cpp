/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "phobos.h"
#include "OpenEntry.h"
#include "VfsRamFifoEntry.h"

using namespace cstd;
using namespace drivers;
using namespace filesystem;
using namespace multitasking;
using namespace middlespace;

/**
 *  Little counter in the right-top corner that indicates the kernel is still alive
 */
void corner_counter() {
    if (auto vga_drv = phobos::driver_manager.get_driver<VgaDriver>()) {
        u64 i = 0;

        while (true) {
            u8 c = (i % 10) + '0';
            vga_drv->char_at(vga_drv->screen_width() - 2, 0) = VgaCharacter { c, EgaColor::White, EgaColor::Black };
            i++;
            Task::yield();
        }
    }
}

u64 start_loading_animation() {
    phobos::printer.print("  loading terminal");
    ktime::OnTimerExpire on_expire = [] () { phobos::printer.print("."); };
    return phobos::time_manager.emplace(1000u, 1000u, on_expire);
}

void stop_loading_animation(u64 timer_id) {
    phobos::time_manager.cancel(timer_id);
}

s32 try_load_and_run_terminal() {
    // open elf file
    auto open_result = phobos::vfs_manager.open("/BIN/TERMINAL");
    if (!open_result) {
        phobos::printer.println(" /BIN/TERMINAL can't be opened. System Halt", EgaColor::Red);
        phobos::halt();
    }
    auto& elf = open_result.value;

    // read elf file data
    u64 size = elf.get_size().value;
    u8* elf_data = new u8[size];
    elf.read(elf_data, size);

    // run the elf
    utils::ElfRunner runner;
    s32 task_id = runner.run(elf_data, new vector<string> { "terminal" });
    switch (task_id) {
    case -ENOMEM:
        phobos::printer.println(" Run failed - no enough memory. System Halt.", EgaColor::Red);
        phobos::halt();
        break;

    case -ENOEXEC:
        phobos::printer.println(" Run failed - not an executable. System Halt.", EgaColor::Red);
        phobos::halt();
        break;

    case -EPERM:
        phobos::printer.println(" Run failed - manager didnt allow. System Halt.", EgaColor::Red);
        phobos::halt();
        break;

    default:
        break;
    }

    return task_id;
}

void wait_terminal_crash(s32 task_id) {
    phobos::task_manager.wait(task_id);
    Task::yield();
}

void print_terminal_crashed() {
    phobos::printer.clear_screen();
    phobos::printer.println("Terminal crashed. Check kernel logs");
    Task::msleep(3000);
}

/**
 * Terminal runner
 */
void run_userspace_terminal() {
    // run in a loop so terminal restarts in case of a crash and kernel log can be read
    while (true) {
        auto timer_id = start_loading_animation();
        auto terminal_task_id = try_load_and_run_terminal();
        stop_loading_animation(timer_id);

        wait_terminal_crash(terminal_task_id);
        print_terminal_crashed();
    }
}

/**
 * Here we enter multitasking
 */
void initial_task() {
//    phobos::task_manager.add_task(TaskFactory::make_kernel_task(corner_counter, "corner_counter"));
    run_userspace_terminal();
}

/**
 * @name    kmain
 * @brief   Kernel entry point, we jump here right from long_mode_init.S
 * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
 */
extern "C" [[noreturn]] void kmain(void* multiboot2_info_ptr) {
    phobos::boot_and_start_multitasking(multiboot2_info_ptr, initial_task);
    __builtin_unreachable();
}
