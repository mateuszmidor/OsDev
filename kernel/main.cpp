/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "GlobalConstructorsRunner.h"
#include "Gdt.h"
#include "KernelLog.h"
#include "Multiboot2.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "PitDriver.h"
#include "Int80hDriver.h"
#include "PCIController.h"
#include "ExceptionManager.h"
#include "VgaDriver.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageFaultHandler.h"
#include "MemoryManager.h"
#include "BumpAllocationPolicy.h"
#include "WyoosAllocationPolicy.h"
#include "SysCallManager.h"
#include "VfsManager.h"
#include "TimeManager.h"
#include "Sse.h"
#include "PageTables.h"
#include "_demos/Demo.h"
#include "_demos/MouseDemo.h"
#include "ElfRunner.h"
#include "SysCallNumbers.h"

using namespace kstd;
using namespace logging;
using namespace drivers;
using namespace cpuexceptions;
using namespace hardware;
using namespace multitasking;
using namespace memory;
using namespace syscalls;
using namespace utils;
using namespace demos;
using namespace filesystem;
using namespace ktime;
using namespace middlespace;

KernelLog& klog                     = KernelLog::instance();
MemoryManager& memory_manager       = MemoryManager::instance();
TaskManager& task_manager           = TaskManager::instance();
DriverManager& driver_manager       = DriverManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();
SysCallManager& syscall_manager     = SysCallManager::instance();
VfsManager& vfs_manager             = VfsManager::instance();
TimeManager& time_manager           = TimeManager::instance();
Gdt                     gdt;
PCIController           pcic;
KeyboardScanCodeSet1    scs1;
KeyboardDriver          keyboard(scs1);
MouseDriver             mouse;
PitDriver               pit;
AtaPrimaryBusDriver     ata_primary_bus;
VgaDriver               vga;
Int80hDriver            int80h;
PageFaultHandler        page_fault;

const u32               PIT_FREQUENCY_HZ = 20;

Key last_key;
TaskList counter_sleep_wl;

struct mode_info_block_t {
    uint16_t mode_attributes;
    uint8_t window_a_attributes;
    uint8_t window_b_attributes;
    uint16_t window_granularity;
    uint16_t window_size;
    uint16_t segment_window_a;
    uint16_t segment_window_b;
    uint16_t window_position_function[2];
    uint16_t pitch; //Bytes per scan line

    //OEM modes

    uint16_t width;
    uint16_t height;
    uint8_t width_char;
    uint8_t height_char;
    uint8_t memory_planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved;

    //VBE 1.2+

    uint8_t red_mask_size;
    uint8_t red_mask_position;
    uint8_t green_mask_size;
    uint8_t green_mask_position;
    uint8_t blue_mask_size;
    uint8_t blue_mask_position;
    uint8_t reserved_mask_size;
    uint8_t reserved__mask_position;
    uint8_t directcolor_attributes;

    //VBE 2.0+

    uint32_t linear_video_buffer;         //LFB (Linear Framebuffer) address
    uint16_t offscreen_memory[2];
    uint16_t offscreen_memory_size;

    uint16_t bytes_per_scan_line;
    uint8_t number_of_images_banked;
    uint8_t number_of_images_linear;
    uint8_t linear_red_mask_size;
    uint8_t linear_red_mask_position;
    uint8_t linear_green_mask_size;
    uint8_t linear_green_mask_position;
    uint8_t linear_blue_mask_size;
    uint8_t linear_blue_mask_position;
    uint8_t linear_reserved_mask_size;
    uint8_t linear_reserved_mask_position;
    uint32_t maximum_pixel_clock;
    uint8_t reserved_2[190];
} __attribute__((packed));


void get_vesa_info() {
    size_t enabled_addr = 0xFFFFFFFF80000000 + 0x9039C;
    bool is_enabled = static_cast<bool>(*reinterpret_cast<uint32_t*>(enabled_addr));

    size_t info_addr = 0xFFFFFFFF80000000 + 0x903A0;
    mode_info_block_t* b = reinterpret_cast<mode_info_block_t*>(info_addr);
    klog.format("VESA: enabled: %, mode: %x%x% bpp\n",
            is_enabled,
            b->width,
            b->height,
            b->bpp
    );
}

VfsEntryPtr keyboard_vfe;
void handle_keyboard_interrupt(const Key &key) {
    if (key != Key::INVALID) {
        last_key = key;

        if (key == Key::F2) {
            if (Task* t = counter_sleep_wl.pop_front())
               task_manager.enqueue_task_back(t);
        }

        // write key to /dev/keyboard RAM file
        if (!keyboard_vfe)
            keyboard_vfe = filesystem::VfsManager::instance().get_entry("/dev/keyboard");

        if (keyboard_vfe) {
            keyboard_vfe->seek(0);
            keyboard_vfe->write(&key, sizeof(key));
        }
    }
}

CpuState* handle_pit_interrupt(CpuState* cpu_state) {
    time_manager.tick();
    return task_manager.schedule(cpu_state);
}

/**
 *  Little counter in the right-top corner
 */
void corner_counter() {
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>()) {
        u64 i = 0;

        while (true) {
            if ((last_key == Key::F1) && (counter_sleep_wl.count() == 0))
                task_manager.dequeue_current_task(counter_sleep_wl);

            u8 c = (i % 10) + '0';
            vga_drv->at(vga_drv->screen_width() - 2, 0) = VgaCharacter { c, EgaColor::White, EgaColor::Black };
            i++;
            Task::yield();
        }
    }
}

void run_userspace_terminal() {
    auto vga_drv = driver_manager.get_driver<VgaDriver>();
    if (!vga_drv)
        return;

    vga_drv->clear_screen();


    VfsEntryPtr e = vfs_manager.get_entry("/BIN/TERMINAL");
    if (!e) {
        vga_drv->print(0, "/BIN/TERMINAL doesnt exist. No terminal will be available", EgaColor::Red);
        return;
    }
    if (e->is_directory()) {
        vga_drv->print(0, "/BIN/TERMINAL is directory? No terminal will be available", EgaColor::Red);
        return;
    }

    vga_drv->print(0, "Loading terminal, please wait a few secs");

    auto on_expire5 = [=]() { vga_drv->print(0, "Loading terminal, please wait a few secs....."); };
    auto on_expire4 = [=]() { vga_drv->print(0, "Loading terminal, please wait a few secs...."); };
    auto on_expire1 = [=]() { vga_drv->print(0, "Loading terminal, please wait a few secs."); };
    auto on_expire2 = [=]() { vga_drv->print(0, "Loading terminal, please wait a few secs.."); };
    auto on_expire3 = [=]() { vga_drv->print(0, "Loading terminal, please wait a few secs..."); };
    time_manager.emplace(1000, on_expire1);
    time_manager.emplace(2000, on_expire2);
    time_manager.emplace(3000, on_expire3);
    time_manager.emplace(4000, on_expire4);
    time_manager.emplace(5000, on_expire5);


    // read elf file data
    u32 size = e->get_size();
    u8* elf_data = new u8[size];
    e->read(elf_data, size);

    // run the elf
    utils::ElfRunner runner;
    if (runner.run(elf_data, new vector<string> { "terminal" }) > 0)
        klog.format("Terminal is running\n");
}

/**
 * Here we enter multitasking
 */
void task_init() {
    task_manager.add_task(TaskFactory::make_kernel_task(Task::idle, "idle"));
    task_manager.add_task(Demo::make_demo<MouseDemo>("mouse", 0));
    task_manager.add_task(TaskFactory::make_kernel_task(corner_counter, "corner_counter"));

    run_userspace_terminal();
}

/**
 * @name    kmain
 * @brief   Kernel entry point, we jump here right from long_mode_init.S
 * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // 0. activate the SSE so the kernel code compiled under -O2 can actually run, remap the kernel to higher half
    Sse::activate_legacy_sse();
    PageTables::map_and_load_kernel_address_space_at_memory_start();

    // 1. initialize multiboot2 info from the data provided by the boot loader
    Multiboot2::initialize(multiboot2_info_ptr);

    // 2. run constructors of global objects
    GlobalConstructorsRunner::run();

    // 3. install new Global Descriptor Table that will allow user-space
    gdt.reinstall_gdt();

    // 4. prepare drivers
    time_manager.set_hz(PIT_FREQUENCY_HZ);
    pit.set_channel0_hz(PIT_FREQUENCY_HZ);
    pit.set_channel0_on_tick(handle_pit_interrupt);
    keyboard.set_on_key_press(handle_keyboard_interrupt);

    // 5. install drivers
    //pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
    driver_manager.install_driver(&keyboard);
    driver_manager.install_driver(&mouse);
    driver_manager.install_driver(&pit);
    driver_manager.install_driver(&ata_primary_bus);
    driver_manager.install_driver(&vga);
    driver_manager.install_driver(&int80h);

    // 6. install exceptions
    exception_manager.install_handler(&page_fault);     // this guy allows dynamic memory allocation

    // 7. configure interrupt manager so that it forwards interrupts and exceptions to proper managers
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts(); // Dynamic memory "new" available from here, as page_fault handler installed and active
    MemoryManager::install_allocation_policy<WyoosAllocationPolicy>(Multiboot2::get_available_memory_first_byte(), Multiboot2::get_available_memory_last_byte());

    // 8. configure and activate system calls through "syscall" instruction
    syscall_manager.config_and_activate_syscalls();

    // 9. install filesystem root "/"
    vfs_manager.install_root();

    // 10. configure vga text mode
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>())
        vga_drv->set_text_mode_90_30();

    get_vesa_info();

    // 11. start multitasking
    task_manager.add_task(TaskFactory::make_kernel_task(task_init, "init"));
    Task::idle();
}
