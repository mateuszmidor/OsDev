/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include <memory>
#include "GlobalConstructorsRunner.h"
#include "Gdt.h"
#include "KernelLog.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "PitDriver.h"
#include "SysCallDriver.h"
#include "PCIController.h"
#include "ExceptionManager.h"
#include "VgaDriver.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageFaultHandler.h"
#include "Terminal.h"
#include "MemoryManager.h"
#include "BumpAllocationPolicy.h"
#include "_demos/Demo.h"
#include "_demos/VgaDemo.h"
#include "_demos/Fat32Demo.h"
#include "_demos/MouseDemo.h"
#include "_demos/MultitaskingDemo.h"
#include "_demos/CpuSpeedDemo.h"


using std::make_shared;
using namespace kstd;
using namespace drivers;
using namespace cpuexceptions;
using namespace hardware;
using namespace multitasking;
using namespace memory;
using namespace utils;
using namespace demos;

Gdt gdt;
PCIController pcic;
MemoryManager& memory_manager       = MemoryManager::instance();
KernelLog& klog                     = KernelLog::instance();
TaskManager& task_manager           = TaskManager::instance();
DriverManager& driver_manager       = DriverManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();

KeyboardScanCodeSet1 scs1;
auto keyboard           = make_shared<KeyboardDriver> (scs1);
auto mouse              = make_shared<MouseDriver>();
auto pit                = make_shared<PitDriver>();
auto ata_primary_bus    = make_shared<AtaPrimaryBusDriver>();
auto sys_call           = make_shared<SysCallDriver>();


/**
 *  Little counter in the right-top corner
 */
void corner_counter(u64 arg) {
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>()) {
        u64 i = 0;

        asm volatile ("syscall" : :"a"(60));  //syscall exit

        while (true) {
            s8 c = (i % 10) + '0';
            vga_drv->at(vga_drv->screen_width() - 2, 0) = VgaCharacter { c, EgaColor::White, EgaColor::Black };
            i++;
            asm volatile("mov $0, %rax; int $0x80");    // yield
        }
    }
}

/**
 * Here we enter multitasking
 */
void task_init(u64 unused) {
    task_manager.add_task(make_shared<Task>(Task::idle, "idle"));
//    task_manager.add_task(Demo::make_demo<MultitaskingDemo>("multitasking_a_demo", 'A'));
//    task_manager.add_task(Demo::make_demo<MultitaskingDemo>("multitasking_b_demo", 'B'));
//    task_manager.add_task(Demo::make_demo<CpuSpeedDemo>("cpuspeed_demo"));
//    task_manager.add_task(Demo::make_demo<Fat32Demo>("fat32_demo"));
    task_manager.add_task(TaskFactory::make<terminal::Terminal>("terminal", 0, false));
    task_manager.add_task(Demo::make_demo<MouseDemo>("mouse_demo", 0, true));
    task_manager.add_task(make_shared<multitasking::Task>(corner_counter, "corner_counter", 0, true));
//    task_manager.add_task(Demo::make_demo<demos::VgaDemo>("vga_demo"));
}

/**
 * @brief   SYSCALL and SYSRET CS and SS selectors
 *          MSR 0xC000_0081
 * @see    AMD64 Architecture Programmer’s Manual Volume 2:System Programming
 *          http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, Figure 6-1. STAR, LSTAR, CSTAR, and MASK MSRs
 */
struct MSR_STAR {
    u32     syscall_target_eip_32bit;   // for calls from 32bit mode
    u16     syscall_cs_ss;              // CS and SS selectors to be loaded during syscall.
                                        // Resulting CS = syscall_cs_ss
                                        // Resulting SS = syscall_cs_ss + 8
                                        // syscall switches to CPL0, so RPL bits must be set to 00b
    u16     sysret_cs_ss;               // CS and SS selectors to be loaded during sysret.
                                        // Resulting CS = sysret_cs_ss + 16.
                                        // Resulting SS = sysret_cs_ss + 8
                                        // sysret returns to CPL3, so RPL bits must be set to 11b
} __attribute__((packed));

/**
 * @brief   "syscall" handler. This is called from syscalls.S
 */
extern "C" void on_syscall(u64 sys_call_num)  {
    klog.format("syscall_handler: % \n", sys_call_num);
}

/**
 * @brief   Raw syscall handler that:
 *          1. saves user task context
 *          2. switches to kernel stack
 *          3. calls on_syscall
 *          4. switches back to user stack
 *          5. restores user task context
 *          implemented in syscalls.S
 */
extern "C" void handle_syscall();

void enable_syscall_sysret() {
    MSR_STAR s_star;
    s_star.syscall_cs_ss = gdt.get_kernel_code_segment_selector() ;
    s_star.sysret_cs_ss = gdt.get_user_data_segment_selector() - 8;
    s_star.syscall_target_eip_32bit = 0;

    u32 mask = 0x200; // disable interrupts
    u64 lstar = (u64)handle_syscall;
    u32 lstar_lo = lstar & 0xFFFFFFFF;
    u32 lstar_hi = lstar >> 32;
    u64 star;
    memcpy(&star, &s_star, sizeof(s_star));
    u32 star_lo = star & 0xFFFFFFFF;
    u32 star_hi = star >> 32;

    // write SFMASK to MSR 0xC000_0084
    // write LSTAR to MSR 0xC000_0082
    // write STAR to MSR 0xC000_0081
    asm volatile (
            // SFMASK
            "mov $0xC0000084, %%ecx     \n;"
            "mov %0, %%rax              \n"
            "wrmsr                      \n"

            // CSTAR
//            "mov $0xC0000083, %%ecx     \n;"
//            "mov %1, %%eax              \n"     // lstar_lo
//            "mov %2, %%edx              \n"     // lstar_hi
//            "wrmsr                      \n"

            // LSTAR
            "mov $0xC0000082, %%ecx     \n;"
            "mov %1, %%eax              \n"     // lstar_lo
            "mov %2, %%edx              \n"     // lstar_hi
            "wrmsr                      \n"

            // STAR
            "mov $0xC0000081, %%ecx     \n;"
            "mov %3, %%eax              \n"     // star_lo
            "mov %4, %%edx              \n"     // star_hi
            "wrmsr                      \n"

            // enable syscall instruction in ESFR Model Specific Register
            "mov $0xC0000080, %%ecx     \n;"
            "rdmsr                      \n;"
            "or $1, %%eax               \n"
            "wrmsr                      \n"
            :
            : "m"(mask), "m"(lstar_lo), "m"(lstar_hi), "m"(star_lo), "m"(star_hi)
            : "memory", "%rax", "%rcx", "%rdx"
    );
}

/**
 * @name    kmain
 * @brief   Kernel entry point, we jump here right from long_mode_init.S
 * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // 0. initialize multiboot2 info from the data provided by the boot loader
    Multiboot2::initialize(multiboot2_info_ptr);

    // 1. setup dynamic memory manager. This must be done before global constructors are run since global constructors may require dynamic memory
    MemoryManager::install_allocation_policy<BumpAllocationPolicy>(Multiboot2::get_available_memory_first_byte(), Multiboot2::get_available_memory_last_byte());

    // 2. run constructors of global objects
    GlobalConstructorsRunner::run();

    // 3. install new Global Descriptor Table that will allow user-space
    gdt.reinstall_gdt();

    // 4. prepare drivers
    pit->set_channel0_hz(20);
    pit->set_channel0_on_tick([](CpuState* cpu_state) { return task_manager.schedule(cpu_state); });

    // 5. install drivers
    pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);
    driver_manager.install_driver(pit);
    driver_manager.install_driver(ata_primary_bus);
    driver_manager.install_driver(sys_call);

    // 6. install exceptions
    exception_manager.install_handler(make_shared<PageFaultHandler>());

    // 7. configure interrupt manager so that it forwards interrupts and exceptions to proper managers
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts();

    // 8. configure vga text mode
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>())
        vga_drv->set_text_mode_90_30();



    // 9. start multitasking
    enable_syscall_sysret();
    task_manager.add_task(make_shared<Task>(task_init, "init"));

    // 10. wait until timer interrupt switches execution to init task
    Task::idle();
}
