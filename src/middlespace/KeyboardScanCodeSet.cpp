/**
 *   @file: ScanCodeSet1.cpp
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#include "KeyboardScanCodeSet.h"
#include "KernelLog.h"
namespace drivers {


KeyboardScanCodeSet1::KeyboardScanCodeSet1 () {
    for (auto& e : basic_code_key)
        e = Key::INVALID;

    basic_code_key[0x01] = Key::Esc;
    basic_code_key[0x02] = (Key)'1';
    basic_code_key[0x03] = (Key)'2';
    basic_code_key[0x04] = (Key)'3';
    basic_code_key[0x05] = (Key)'4';
    basic_code_key[0x06] = (Key)'5';
    basic_code_key[0x07] = (Key)'6';
    basic_code_key[0x08] = (Key)'7';
    basic_code_key[0x09] = (Key)'8';
    basic_code_key[0x0A] = (Key)'9';
    basic_code_key[0x0B] = (Key)'0';
    basic_code_key[0x0C] = (Key)'-';
    basic_code_key[0x0D] = (Key)'=';
    basic_code_key[0x0E] = Key::Backspace;
    basic_code_key[0x0F] = Key::Tab;
    basic_code_key[0x10] = (Key)'q';
    basic_code_key[0x11] = (Key)'w';
    basic_code_key[0x12] = (Key)'e';
    basic_code_key[0x13] = (Key)'r';
    basic_code_key[0x14] = (Key)'t';
    basic_code_key[0x15] = (Key)'y';
    basic_code_key[0x16] = (Key)'u';
    basic_code_key[0x17] = (Key)'i';
    basic_code_key[0x18] = (Key)'o';
    basic_code_key[0x19] = (Key)'p';
    basic_code_key[0x1A] = (Key)'[';
    basic_code_key[0x1B] = (Key)']';
    basic_code_key[0x1C] = Key::Enter;
    basic_code_key[0x1D] = Key::LCtrl;
    basic_code_key[0x1E] = (Key)'a';
    basic_code_key[0x1F] = (Key)'s';
    basic_code_key[0x20] = (Key)'d';
    basic_code_key[0x21] = (Key)'f';
    basic_code_key[0x22] = (Key)'g';
    basic_code_key[0x23] = (Key)'h';
    basic_code_key[0x24] = (Key)'j';
    basic_code_key[0x25] = (Key)'k';
    basic_code_key[0x26] = (Key)'l';
    basic_code_key[0x27] = (Key)';';
    basic_code_key[0x28] = (Key)'\'';
    basic_code_key[0x29] = (Key)'`';
    basic_code_key[0x2A] = Key::LShift;
    basic_code_key[0x2B] = (Key)'\\';
    basic_code_key[0x2C] = (Key)'z';
    basic_code_key[0x2D] = (Key)'x';
    basic_code_key[0x2E] = (Key)'c';
    basic_code_key[0x2F] = (Key)'v';
    basic_code_key[0x30] = (Key)'b';
    basic_code_key[0x31] = (Key)'n';
    basic_code_key[0x32] = (Key)'m';
    basic_code_key[0x33] = (Key)',';
    basic_code_key[0x34] = (Key)'.';
    basic_code_key[0x35] = (Key)'/';
    basic_code_key[0x37] = (Key)'*';
    basic_code_key[0x38] = Key::LAlt;
    basic_code_key[0x39] = (Key)' ';
    basic_code_key[0x3A] = Key::CapsLock;
    basic_code_key[0x3B] = Key::F1;
    basic_code_key[0x3C] = Key::F2;
    basic_code_key[0x3D] = Key::F3;
    basic_code_key[0x3E] = Key::F4;
    basic_code_key[0x3F] = Key::F5;
    basic_code_key[0x40] = Key::F6;
    basic_code_key[0x41] = Key::F7;
    basic_code_key[0x42] = Key::F8;
    basic_code_key[0x43] = Key::F9;
    basic_code_key[0x44] = Key::F10;
    basic_code_key[0x57] = Key::F11;
    basic_code_key[0x58] = Key::F12;
    basic_code_key[0x47] = (Key)'7';
    basic_code_key[0x48] = (Key)'8';
    basic_code_key[0x49] = (Key)'9';
    basic_code_key[0x4A] = (Key)'-';
    basic_code_key[0x4B] = (Key)'4';
    basic_code_key[0x4C] = (Key)'5';
    basic_code_key[0x4D] = (Key)'6';
    basic_code_key[0x4E] = (Key)'+';
    basic_code_key[0x4F] = (Key)'1';
    basic_code_key[0x50] = (Key)'2';
    basic_code_key[0x51] = (Key)'3';
    basic_code_key[0x52] = (Key)'0';
    basic_code_key[0x53] = (Key)'.';
    basic_code_key[0x9D] = Key::LCtrl_Released;
    basic_code_key[0xAA] = Key::LShift_Released;
    basic_code_key[0xB8] = Key::LAlt_Released;


    for (auto& e : basic_shift_code_key)
        e = Key::INVALID;

    basic_shift_code_key[0x01] = Key::Esc;
    basic_shift_code_key[0x02] = (Key)'!';
    basic_shift_code_key[0x03] = (Key)'@';
    basic_shift_code_key[0x04] = (Key)'#';
    basic_shift_code_key[0x05] = (Key)'$';
    basic_shift_code_key[0x06] = (Key)'%';
    basic_shift_code_key[0x07] = (Key)'^';
    basic_shift_code_key[0x08] = (Key)'&';
    basic_shift_code_key[0x09] = (Key)'*';
    basic_shift_code_key[0x0A] = (Key)'(';
    basic_shift_code_key[0x0B] = (Key)')';
    basic_shift_code_key[0x0C] = (Key)'_';
    basic_shift_code_key[0x0D] = (Key)'+';
    basic_shift_code_key[0x0E] = Key::Backspace;
    basic_shift_code_key[0x0F] = Key::Tab;
    basic_shift_code_key[0x10] = (Key)'Q';
    basic_shift_code_key[0x11] = (Key)'W';
    basic_shift_code_key[0x12] = (Key)'E';
    basic_shift_code_key[0x13] = (Key)'R';
    basic_shift_code_key[0x14] = (Key)'T';
    basic_shift_code_key[0x15] = (Key)'Y';
    basic_shift_code_key[0x16] = (Key)'U';
    basic_shift_code_key[0x17] = (Key)'I';
    basic_shift_code_key[0x18] = (Key)'O';
    basic_shift_code_key[0x19] = (Key)'P';
    basic_shift_code_key[0x1A] = (Key)'{';
    basic_shift_code_key[0x1B] = (Key)'}';
    basic_shift_code_key[0x1C] = Key::Enter;
    basic_shift_code_key[0x1D] = Key::LCtrl;
    basic_shift_code_key[0x1E] = (Key)'A';
    basic_shift_code_key[0x1F] = (Key)'S';
    basic_shift_code_key[0x20] = (Key)'D';
    basic_shift_code_key[0x21] = (Key)'F';
    basic_shift_code_key[0x22] = (Key)'G';
    basic_shift_code_key[0x23] = (Key)'H';
    basic_shift_code_key[0x24] = (Key)'J';
    basic_shift_code_key[0x25] = (Key)'K';
    basic_shift_code_key[0x26] = (Key)'L';
    basic_shift_code_key[0x27] = (Key)':';
    basic_shift_code_key[0x28] = (Key)'"';
    basic_shift_code_key[0x29] = (Key)'~';
    basic_shift_code_key[0x2A] = Key::LShift;
    basic_shift_code_key[0x2B] = (Key)'|';
    basic_shift_code_key[0x2C] = (Key)'Z';
    basic_shift_code_key[0x2D] = (Key)'X';
    basic_shift_code_key[0x2E] = (Key)'C';
    basic_shift_code_key[0x2F] = (Key)'V';
    basic_shift_code_key[0x30] = (Key)'B';
    basic_shift_code_key[0x31] = (Key)'N';
    basic_shift_code_key[0x32] = (Key)'M';
    basic_shift_code_key[0x33] = (Key)'<';
    basic_shift_code_key[0x34] = (Key)'>';
    basic_shift_code_key[0x35] = (Key)'?';
    basic_shift_code_key[0x37] = (Key)'*';
    basic_shift_code_key[0x38] = Key::LAlt;
    basic_shift_code_key[0x39] = (Key)' ';
    basic_shift_code_key[0x3A] = Key::CapsLock;
    basic_shift_code_key[0x3B] = Key::F1;
    basic_shift_code_key[0x3C] = Key::F2;
    basic_shift_code_key[0x3D] = Key::F3;
    basic_shift_code_key[0x3E] = Key::F4;
    basic_shift_code_key[0x3F] = Key::F5;
    basic_shift_code_key[0x40] = Key::F6;
    basic_shift_code_key[0x41] = Key::F7;
    basic_shift_code_key[0x42] = Key::F8;
    basic_shift_code_key[0x43] = Key::F9;
    basic_shift_code_key[0x44] = Key::F10;
    basic_shift_code_key[0x57] = Key::F11;
    basic_shift_code_key[0x58] = Key::F12;
    basic_shift_code_key[0x47] = (Key)'7';
    basic_shift_code_key[0x48] = (Key)'8';
    basic_shift_code_key[0x49] = (Key)'9';
    basic_shift_code_key[0x4A] = (Key)'-';
    basic_shift_code_key[0x4B] = (Key)'4';
    basic_shift_code_key[0x4C] = (Key)'5';
    basic_shift_code_key[0x4D] = (Key)'6';
    basic_shift_code_key[0x4E] = (Key)'+';
    basic_shift_code_key[0x4F] = (Key)'1';
    basic_shift_code_key[0x50] = (Key)'2';
    basic_shift_code_key[0x51] = (Key)'3';
    basic_shift_code_key[0x52] = (Key)'0';
    basic_shift_code_key[0x53] = (Key)'.';
    basic_shift_code_key[0x9D] = Key::LCtrl_Released;
    basic_shift_code_key[0xAA] = Key::LShift_Released;
    basic_shift_code_key[0xB8] = Key::LAlt_Released;


    for (auto& e : extended_code_key)
        e = Key::INVALID;

    extended_code_key[0x1C] = Key::Enter;
    extended_code_key[0x35] = (Key)'/';
    extended_code_key[0x47] = Key::Home;
    extended_code_key[0x4F] = Key::End;
    extended_code_key[0x48] = Key::Up;
    extended_code_key[0x50] = Key::Down;
    extended_code_key[0x4B] = Key::Left;
    extended_code_key[0x4D] = Key::Right;
    extended_code_key[0x49] = Key::PgUp;
    extended_code_key[0x51] = Key::PgDown;
}


/**
 * @brief   Consume key scan code and return key if code sequence has ended or INVALID otherwise
 * @param   key_code Raw keyboard scan code as received from the keyboard
 */
const Key KeyboardScanCodeSet1::push_code(u8 key_code) {
    // debug
//    KernelLog& klog = KernelLog::instance();
//    klog.format("0x%\n", kstd::to_str(key_code, 16));

    Key result;

    // extended key; some keys are made of sequences of key codes;
    // usually only 2 key codes: 0xE0 and then the actual extended key code
    if (extended_key_incoming) {
        result = extended_code_key[key_code];
        extended_key_incoming = false;
    }
    // basic key
    else {
        if (key_code == EXTENDED_KEY_SEQUENCE_BEGIN) {
            extended_key_incoming = true;
            return Key::INVALID;
        }

        if (is_lshift_down xor is_caps_active)
            result = basic_shift_code_key[key_code];
        else
            result = basic_code_key[key_code];

        switch (result) {
        case Key::LShift:
            is_lshift_down = true;
            break;
        case Key::LShift_Released:
            is_lshift_down = false;
            break;
        case Key::CapsLock:
            is_caps_active = !is_caps_active;
//            klog.format("caps: %\n", is_caps_active);
            break;

        default:;
        }
    }

    return result;
}

}   /* namespace drivers */
