/**
 *   @file: phobos.cpp
 *
 *   @date: Jan 23, 2018
 * @author: Mateusz Midor
 */

#include "Sse.h"
#include "Multiboot2.h"
#include "GlobalConstructorsRunner.h"
#include "Gdt.h"
#include "InterruptManager.h"
#include "ExceptionManager.h"
#include "SysCallManager.h"
#include "MemoryManager.h"
#include "KeyboardDriver.h"
#include "KeyboardScanCodeSet.h"
#include "Mouse.h"
#include "MouseDriver.h"
#include "PitDriver.h"
#include "Int80hDriver.h"
#include "PCIController.h"
#include "PageFaultHandler.h"
#include "PageTables.h"
#include "BumpAllocationPolicy.h"
#include "WyoosAllocationPolicy.h"
#include "Assert.h"
#include "KernelLog.h"

#include "VfsRamMountPoint.h"
#include "VfsRamDirectoryEntry.h"
#include "VfsRamFifoEntry.h"
#include "VfsKmsgEntry.h"
#include "VfsMemInfoEntry.h"
#include "VfsDateEntry.h"
#include "VfsCpuInfoEntry.h"
#include "VfsPciInfoEntry.h"
#include "VfsPsInfoEntry.h"
#include "VfsMountInfoEntry.h"
#include "MassStorageMsDos.h"
#include "VfsFat32MountPoint.h"

#include "phobos.h"


using namespace cstd;
using namespace logging;
using namespace drivers;
using namespace cpuexceptions;
using namespace hardware;
using namespace multitasking;
using namespace memory;
using namespace syscalls;
using namespace utils;
using namespace filesystem;
using namespace ktime;
using namespace middlespace;

namespace phobos {
    namespace details {
        constexpr u32           PIT_FREQUENCY_HZ    {20};
        KernelLog&              klog                {logging::KernelLog::instance()};
        MemoryManager&          memory_manager      {memory::MemoryManager::instance()};
        ExceptionManager&       exception_manager   {cpuexceptions::ExceptionManager::instance()};
        InterruptManager&       interrupt_manager   {hardware::InterruptManager::instance()};
        SysCallManager&         syscall_manager     {syscalls::SysCallManager::instance()};
        Gdt                     gdt;
        PCIController           pcic;
        KeyboardScanCodeSet1    scs1;
        KeyboardDriver          keyboard            {scs1};
        MouseDriver             mouse;
        PitDriver               pit;
        AtaPrimaryBusDriver     ata_primary_bus;
        AtaSecondaryBusDriver   ata_secondary_bus;
        VgaDriver               vga;
        Int80hDriver            int80h;
        PageFaultHandler        page_fault;

        /**
         * @brief   Install ram fs /dev
         */
        void install_dev_fs() {
            auto dev = std::make_shared<VfsRamDirectoryEntry>("dev");
            dev->attach_entry(std::make_shared<VfsRamFifoEntry>("mouse"));
            dev->attach_entry(std::make_shared<VfsRamFifoEntry>("keyboard"));
            vfs_manager.attach("/", dev);
        }

        /**
         * @brief   Install ram fs /proc
         */
        void install_proc_fs() {
            vfs_manager.attach("/", std::make_shared<VfsRamMountPoint>("proc"));
            vfs_manager.attach("/proc", std::make_shared<VfsKmsgEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsMemInfoEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsDateEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsCpuInfoEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsPciInfoEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsPsInfoEntry>());
            vfs_manager.attach("/proc", std::make_shared<VfsMountInfoEntry>());
        }

        /**
         * @brief   Mount all volumes available in "hdd" under the root
         */
        void mount_hdd_fat32_volumes(const AtaDevice& hdd) {
            if (!MassStorageMsDos::verify(hdd))
                return;

            MassStorageMsDos ms(hdd);
            for (const auto& v : ms.get_volumes())
                vfs_manager.attach("/", std::make_shared<VfsFat32MountPoint>(v));
        }

        /**
         * @brief   Install all available fat32 volumes under "/"
         */
        void install_fat32_fs() {
            if (const auto& ata_primary_bus = driver_manager.get_driver<AtaPrimaryBusDriver>()) {
                if (ata_primary_bus->master_hdd.is_present()) {
                    mount_hdd_fat32_volumes(ata_primary_bus->master_hdd);
                }

                if (ata_primary_bus->slave_hdd.is_present()) {
                    mount_hdd_fat32_volumes(ata_primary_bus->slave_hdd);
                }
            }

            if (const auto&  ata_secondary_bus = driver_manager.get_driver<AtaSecondaryBusDriver>()) {
                if (ata_secondary_bus->master_hdd.is_present()) {
                    mount_hdd_fat32_volumes(ata_secondary_bus->master_hdd);
                }

                if (ata_secondary_bus->slave_hdd.is_present()) {
                    mount_hdd_fat32_volumes(ata_secondary_bus->slave_hdd);
                }
            }
        }

        /**
         * @brief   Keyboard handling
         */
        OpenEntry keyboard_vfe;

        /**
         * @brief   Interrupt handling. No blocking allowed
         */
        void handle_key_press(const Key &key) {
            constexpr u32 MAX_KEYS_IN_FILE {10};

            // write key to /dev/keyboard RAM file
            if (!keyboard_vfe)
                keyboard_vfe = vfs_manager.open("/dev/keyboard").value;

            if (keyboard_vfe) {
                // dont let the keyboard file overflow and block (we are in interrupt not task context)
                if (keyboard_vfe.get_size().value > sizeof(key) * MAX_KEYS_IN_FILE)
                    keyboard_vfe.truncate(sizeof(key) * MAX_KEYS_IN_FILE);
                keyboard_vfe.write(&key, sizeof(key));
            }
        }

        /**
         * @brief   Mouse handling
         */
        middlespace::MouseState mouse_state;
        OpenEntry mouse_vfe;

        /**
         * @brief   Interrupt handling. No blocking allowed
         */
        void update_mouse() {
            constexpr u32 MAX_STATES_IN_FILE {10};

            // write key to /dev/mouse RAM file
            if (!mouse_vfe)
                mouse_vfe = vfs_manager.open("/dev/mouse").value;

            if (mouse_vfe) {
                // dont let the mouse state file overflow and block (we are in interrupt not task context)
                if (mouse_vfe.get_size().value > sizeof(mouse_state) * MAX_STATES_IN_FILE)
                    mouse_vfe.truncate(sizeof(mouse_state) * MAX_STATES_IN_FILE);
                mouse_vfe.write(&mouse_state, sizeof(mouse_state));
            }
        }

        void handle_mouse_down(MouseButton button) {
            mouse_state.buttons[button] = true;
            update_mouse() ;
        }

        void handle_mouse_up(MouseButton button) {
            mouse_state.buttons[button] = false;
            update_mouse() ;
        }

        void handle_mouse_move(s8 dx, s8 dy) {
            mouse_state.dx = dx;
            mouse_state.dy = dy;
            update_mouse() ;
        }

        /**
         * @brief   Programmable Interval Timer handling
         */
        CpuState* handle_timer_tick(CpuState* cpu_state) {
            time_manager.tick();
            return task_manager.schedule(cpu_state);
        }

        /**
         * @brief    Print the OS loading header
         */
        void print_phobos_loading() {
            printer.println(R"( ____  _           _      ___  ____  )");
            printer.println(R"(|  _ \| |__   ___ | |__  / _ \/ ___| )");
            printer.println(R"(| |_) | '_ \ / _ \| '_ \| | | \___ \ )");
            printer.println(R"(|  __/| | | | (_) | |_) | |_| |___) |)");
            printer.println(R"(|_|   |_| |_|\___/|_.__/ \___/|____/ )");
            printer.println("");
            printer.println("Loading");
        }
    } // namespace details


    /**
     * Public phobos interface
     */
    DriverManager&  driver_manager          {drivers::DriverManager::instance()};
    TimeManager&    time_manager            {ktime::TimeManager::instance()};
    TaskManager&    task_manager            {multitasking::TaskManager::instance()};
    VfsManager&     vfs_manager             {filesystem::VfsManager::instance()};
    VgaPrinter      printer                 {details::vga};


    /**
     * @brief   Boot the kernel and enter multitasking starting at "init_task"
     * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
     */
    [[noreturn]] void boot_and_start_multitasking(void* multiboot2_info_ptr, const InitTaskPtr init_task) {
        using namespace details;

        // 0. activate the SSE so the kernel code compiled under -O2 can actually run, remap the kernel to higher half
        Sse::activate_legacy_sse();
        PageTables::map_and_load_kernel_address_space();

        // 1. initialize multiboot2 info from the data provided by the boot loader
        Multiboot2::initialize(multiboot2_info_ptr);

        // 2. run constructors of global objects
        GlobalConstructorsRunner::run();

        // 3. configure temporary console and print PhobOS logo & loading message
        print_phobos_loading();

        // 4. install new Global Descriptor Table that will allow user-space
        gdt.reinstall_gdt();
        printer.println("  installing GDT...done");

        // 5. prepare drivers
        time_manager.set_hz(PIT_FREQUENCY_HZ);
        pit.set_channel0_hz(PIT_FREQUENCY_HZ);
        pit.set_channel0_on_tick(handle_timer_tick);
        keyboard.set_on_key_press(handle_key_press);
        mouse.set_on_down(handle_mouse_down);
        mouse.set_on_up(handle_mouse_up);
        mouse.set_on_move(handle_mouse_move);

        // 6. install drivers
        //pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
        driver_manager.install_driver(&keyboard);
        driver_manager.install_driver(&mouse);
        driver_manager.install_driver(&pit);
        driver_manager.install_driver(&ata_primary_bus);
        driver_manager.install_driver(&ata_secondary_bus);
        driver_manager.install_driver(&vga);
        driver_manager.install_driver(&int80h);
        printer.println("  installing drivers...done");

        // 7. install exceptions
        exception_manager.install_handler(&page_fault);    // this guy allows dynamic memory on-page-fault allocation
        printer.println("  installing exceptions...done");

        // 8. configure interrupt manager so that it forwards interrupts and exceptions to proper managers
        interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
        interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
        interrupt_manager.config_and_activate_exceptions_and_interrupts(); // on-page-fault allocation available from here, as page_fault handler installed and activated
        printer.println("  installing interrupts...done");

        // 9. configure dynamic memory management
        MemoryManager::install_allocation_policy<WyoosAllocationPolicy>(Multiboot2::get_available_memory_first_byte(), Multiboot2::get_available_memory_last_byte());
        printer.println("  installing dynamic memory...done");

        // 10. configure and activate system calls through "syscall" instruction
        syscall_manager.config_and_activate_syscalls();
        printer.println("  installing system calls...done");

        // 11. install filesystems
        vfs_manager.install();
        install_dev_fs();
        install_proc_fs();
        install_fat32_fs();
        printer.println("  installing virtual file system...done");

        // 12. start multitasking
        task_manager.install_multitasking();
        task_manager.add_task(TaskFactory::make_kernel_task(init_task, "init"));
        printer.println("  installing multitasking...done");
        Task::idle();

        // 13. we nerver get past Task::idle()
        __builtin_unreachable();
    }

    /**
     * @brief   Halt the system. Use it in case of boot error.
     */
    [[noreturn]] void halt() {
        utils::phobos_halt();
        __builtin_unreachable();
    }
}

/**
 * Necessary memory management global symbols
 */
void* operator new(size_t size) {
    return phobos::details::memory_manager.alloc_virt_memory(size);
}

void* operator new[](size_t size) {
    return phobos::details::memory_manager.alloc_virt_memory(size);
}

void operator delete[](void* ptr) {
    phobos::details::memory_manager.free_virt_memory(ptr);
}

void operator delete(void *ptr) {
    phobos::details::memory_manager.free_virt_memory(ptr);
}
