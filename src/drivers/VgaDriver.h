/**
 *   @file: VgaDriver.h
 *
 *   @date: Jun 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_VGADRIVER_H_
#define SRC_DRIVERS_VGADRIVER_H_

#include "DeviceDriver.h"
#include "Port.h"

namespace drivers {

// ega 16 color pallete index, mode that GRUB lefts us in
enum EgaColor : u8 {
    Black      = 0,
    Blue       = 1,
    Green      = 2,
    Cyan       = 3,
    Red        = 4,
    Magenta    = 5,
    Brown      = 6,
    LightGray  = 7,
    DarkGray   = 8,
    LightBlue  = 9,
    LightGreen = 10,
    LightCyan  = 11,
    LightRed   = 12,
    Pink       = 13,
    Yellow     = 14,
    White      = 15
};

class VgaDriver : public DeviceDriver {
public:
    s16 handled_interrupt_no() override;
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) override;

    void set_text_mode_90_30();
    void set_graphics_mode_320_200_256();
    void put_pixel(u16 x, u16 y, u8 color_index) const;
    u16 screen_width() const;
    u16 screen_height() const;

private:
    void write_registers(u8* registers) const;
    u8* get_framebuffer_segment() const;

    u16 width   {80};  // in characters (text mode)
    u16 height  {25};  // in characters (text mode)
    Port8bit misc_port                          {0x3C2};
    Port8bit crtc_index_port                    {0x3D4};
    Port8bit crtc_data_port                     {0x3D5};
    Port8bit sequence_index_port                {0x3C4};
    Port8bit sequence_data_port                 {0x3C5};
    Port8bit graphics_controller_index_port     {0x3CE};
    Port8bit graphics_controller_data_port      {0x3CF};
    Port8bit attribute_controller_index_port    {0x3C0};
    Port8bit attribute_controller_read_port     {0x3C1};
    Port8bit attribute_controller_write_port    {0x3C0};
    Port8bit attribute_controller_reset_port    {0x3DA};
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_VGADRIVER_H_ */
