/**
 *   @file: VgaDriver.h
 *
 *   @date: Jun 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_VGADRIVER_H_
#define SRC_DRIVERS_VGADRIVER_H_

#include "VgaCharacter.h"
#include "DeviceDriver.h"
#include "Port.h"

namespace drivers {

class VgaDriver : public DeviceDriver {
public:
    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;

    void set_text_mode_80_25();
    void set_text_mode_90_30();
    void set_graphics_mode_320_200_256();
    void put_pixel(u16 x, u16 y, u8 color_index) const;
    VgaCharacter& at(u16 x, u16 y) const;
    void flush_buffer(const VgaCharacter* buff);
    void set_cursor_visible(bool visible);
    void set_cursor_pos(u8 x, u8 y);
    u16 screen_width() const;
    u16 screen_height() const;

    // higher level basic functions
    void clear_screen(EgaColor color = EgaColor::Black);
    void print(u16 y, const char* text, EgaColor fg = EgaColor::White, EgaColor bg = EgaColor::Black);

private:
    void write_registers(u8* registers) const;
    u8* get_framebuffer_segment() const;

    VgaCharacter* const vga                             {(VgaCharacter*)0xFFFFFFFF800b8000};
    u16 width                                           {80};  // in characters (text mode)
    u16 height                                          {25};  // in characters (text mode)
    // text mode
    hardware::Port8bit cursor_cmd_port                  {0x3D4};
    hardware::Port8bit cursor_data_port                 {0x3D5};
    // graphics mode
    hardware::Port8bit misc_port                        {0x3C2};
    hardware::Port8bit crtc_index_port                  {0x3D4};
    hardware::Port8bit crtc_data_port                   {0x3D5};
    hardware::Port8bit sequence_index_port              {0x3C4};
    hardware::Port8bit sequence_data_port               {0x3C5};
    hardware::Port8bit graphics_controller_index_port   {0x3CE};
    hardware::Port8bit graphics_controller_data_port    {0x3CF};
    hardware::Port8bit attribute_controller_index_port  {0x3C0};
    hardware::Port8bit attribute_controller_read_port   {0x3C1};
    hardware::Port8bit attribute_controller_write_port  {0x3C0};
    hardware::Port8bit attribute_controller_reset_port  {0x3DA};
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_VGADRIVER_H_ */
