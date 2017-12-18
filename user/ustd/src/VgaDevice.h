/**
 *   @file: VgaDevice.h
 *
 *   @date: Dec 15, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_VGADEVICE_H_
#define USER_USTD_SRC_VGADEVICE_H_


#include "Vga.h"
#include "Vector.h"

namespace ustd {

/**
 * @brief   This class provides VGA graphics drawing capabilities
 */
class VgaDevice {
public:
    VgaDevice();
    ~VgaDevice();
    void set_pixel_at(s16 x, s16 y, middlespace::EgaColor64 color);
    void draw_rect(u16 x, u16 y, u16 width, u16 height, middlespace::EgaColor64 color);
    void draw_rect(u16 x, u16 y, u16 width, u16 height, middlespace::EgaColor64 frame, middlespace::EgaColor64 body);
    void draw_circle(u16 x, u16 y, s16 radius, middlespace::EgaColor64 color);
    void draw_text(u16 px, u16 py, const char* s, middlespace::EgaColor64 color);
    void flush_to_screen() const;

    u16 width, height;

private:
    void draw_char_8x8(u16 px, u16 py, char c, middlespace::EgaColor64 color);

    static unsigned char font_8x8[2048];
    vector<middlespace::EgaColor64> vga_buffer;

};


} /* namespace ustd */

#endif /* USER_USTD_SRC_VGADEVICE_H_ */
