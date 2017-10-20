/**
 *   @file: SysCallManager.h
 *
 *   @date: Sep 13, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_SYSCALLS_SYSCALLMANAGER_H_
#define SRC_SYSCALLS_SYSCALLMANAGER_H_

namespace syscalls {

/**
 * @brief   SYSCALL and SYSRET CS and SS selectors
 *          Written to MSR 0xC000_0081
 * @see     AMD64 Architecture Programmer’s Manual Volume 2:System Programming
 *          http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, Figure 6-1. STAR, LSTAR, CSTAR, and MASK MSRs
 */
struct MSR_STAR {
    u32     syscall_target_eip_32bit;   // reserved for syscall s from 32bit compatibility mode
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
 * @brief   This class configures and installs handlers for system calls invoked through "syscall" instruction
 * @see     http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, 6.1.1 SYSCALL and SYSRET
 *          https://software.intel.com/sites/default/files/managed/7c/f1/253668-sdm-vol-3a.pdf, 5.8.8 Fast System Calls in 64-Bit Mode
 */
class SysCallManager {
public:
    static SysCallManager& instance();

    void config_and_activate_syscalls();
    s32 file_open(const char* name, u32 flags, u32 mode);
    s32 file_close(u32 fd);
    ssize_t file_read(int fd, void *buf, size_t count);
    ssize_t file_write(int fd, const void *buf, size_t count);

    void vga_cursor_setvisible(bool visible);
    void vga_cursor_setpos(u8 x, u8 y);
    void vga_setat(u8 x, u8 y, u16 c);

private:
    static SysCallManager _instance;

    SysCallManager() {}
    SysCallManager operator=(const SysCallManager&) = delete;
    SysCallManager operator=(SysCallManager&&) = delete;

};

} /* namespace syscalls */

#endif /* SRC_SYSCALLS_SYSCALLMANAGER_H_ */
