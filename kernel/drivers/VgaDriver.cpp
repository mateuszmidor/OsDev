/**
 *   @file: VgaDriver.cpp
 *
 *   @date: Jun 26, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "VgaDriver.h"

using middlespace::EgaColor;
using middlespace::VgaCharacter;
using namespace hardware;
namespace drivers {

s16 VgaDriver::handled_interrupt_no() {
    return Interrupts::Vga;
}

CpuState* VgaDriver::on_interrupt(CpuState* cpu_state) {
    return cpu_state;
}

void VgaDriver::set_text_mode_90_30() {
    if (width == 90 && height == 30)
        return;

    // restore framebuffer_segment contents; seems that VGA font is keept there
    if (framebuffer_segment && framebuffer_segment_copy) {
        memcpy(framebuffer_segment, framebuffer_segment_copy, width * height);
        delete[] framebuffer_segment_copy;
        framebuffer_segment_copy = nullptr;
    }

    // http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
    u8 g_90x30_text[] =
    {
    /* MISC */
        0xE7,
    /* SEQ */
        0x03, 0x01, 0x03, 0x00, 0x02,
    /* CRTC */
        0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
        0xEA, 0x0C, 0xDF, 0x2D, 0x10, 0xE8, 0x05, 0xA3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00,
    };

    write_registers(g_90x30_text);

    width = 90;
    height = 30;
}

void VgaDriver::set_graphics_mode_320_200_256() {
    if (width == 320 && height == 200)
        return;

    // http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
    u8 g_320x200x256[] =
    {
    /* MISC */
        0x63,
    /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };

    write_registers(g_320x200x256);
    width = 320;
    height = 200;

    // make copy of framebuffer_segment; seems that VGA font is keept there
    framebuffer_segment = get_framebuffer_segment();
    framebuffer_segment_copy = new u8[width*height];
    memcpy(framebuffer_segment_copy, framebuffer_segment, width*height);
}

void VgaDriver::put_pixel(u16 x, u16 y, u8 color_index) const {
    if (x >= width || y >= height || !framebuffer_segment)
        return;

    u8* ptr = framebuffer_segment + y * width + x;
    *ptr = color_index;
}

VgaCharacter& VgaDriver::at(u16 x, u16 y) const {
    if ((x >= width) || (y >= height))
        return vga[0];

    return vga[y * width + x];
}

void VgaDriver::flush_buffer(const VgaCharacter* buff) {
    memcpy(vga, buff, screen_width() * screen_height() * sizeof(VgaCharacter));
}

/**
 * @ref http://wiki.osdev.org/Text_Mode_Cursor#Source_in_C_2
 */
void VgaDriver::set_cursor_visible(bool visible) {
    const u8 CURSOR_HIDDEN = 32; // bit5

    cursor_cmd_port.write(0x0A);
    u8 cursor_state = cursor_data_port.read();
    if (visible)
        cursor_state &= ~CURSOR_HIDDEN;
    else
        cursor_state |= CURSOR_HIDDEN;
    cursor_data_port.write(cursor_state);
}

/**
 * @ref http://wiki.osdev.org/Text_Mode_Cursor#Source_in_C
 */
void VgaDriver::set_cursor_pos(u8 x, u8 y) {
   u16 position = y * width + x;

   // cursor LOW port to vga INDEX register
   cursor_cmd_port.write(0x0F);
   cursor_data_port.write(position & 0xFF);
   // cursor HIGH port to vga INDEX register
   cursor_cmd_port.write(0x0E);
   cursor_data_port.write(position >> 8);
}

/**
 * screen_width
 * @return  Num columns (text mode), num pixels (graphics mode)
 */
u16 VgaDriver::screen_width() const {
    return width;
}

/**
 *
 * @return  Num rows (text mode), num pixels (graphics mode)
 */
u16 VgaDriver::screen_height() const {
    return height;
}

void VgaDriver::clear_screen(EgaColor color) {
    for (u16 y = 0; y < height; y++)
        for (u16 x = 0; x < width; x++)
            at(x, y) = VgaCharacter(' ', color, color);
}

void VgaDriver::print(u16 y, const char* text,  EgaColor fg, EgaColor bg) {
    u32 curr = y * width;
    u32 max_index = width * height;

    while (*text && curr < max_index) {
        vga[curr] = VgaCharacter(*text, fg, bg);
        text++;
        curr++;
    }
}

void VgaDriver::write_registers(u8* registers) const {
    // MISC
    misc_port.write(*(registers++));

    // SEQ
    for (u8 i = 0; i < 5; i++) {
        sequence_index_port.write(i);
        sequence_data_port.write(*(registers++));
    }

    // CRTC (cathode ray tube controller)
    // lock crtc before configuring
    crtc_index_port.write(0x03);
    crtc_data_port.write(crtc_data_port.read() | 0x80);
    crtc_index_port.write(0x11);
    crtc_data_port.write(crtc_data_port.read() & ~0x80);
    registers[0x03] |= 0x80;
    registers[0x11] &= ~0x80;
    for (u8 i = 0; i < 25; i++) {
        crtc_index_port.write(i);
        crtc_data_port.write(*(registers++));
    }

    // GC (graphics controller)
    for (u8 i = 0; i < 9; i++) {
        graphics_controller_index_port.write(i);
        graphics_controller_data_port.write(*(registers++));
    }

    // AC (attribute controller)
    for (u8 i = 0; i < 21; i++) {
        attribute_controller_reset_port.read();
        attribute_controller_index_port.write(i);
        attribute_controller_write_port.write(*(registers++));
    }

    attribute_controller_reset_port.read();
    attribute_controller_index_port.write(0x20);
}

u8* VgaDriver::get_framebuffer_segment() const {
    graphics_controller_index_port.write(0x06);
    u8 segment_no = (graphics_controller_data_port.read() >> 2) & 0x03;
    switch(segment_no) {
        default:
        case 0: return (u8*)0xFFFFFFFF80000000;
        case 1: return (u8*)0xFFFFFFFF800A0000;
        case 2: return (u8*)0xFFFFFFFF800B0000;
        case 3: return (u8*)0xFFFFFFFF800B8000;
    }
}

} /* namespace drivers */
