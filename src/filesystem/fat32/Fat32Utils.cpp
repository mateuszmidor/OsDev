/**
 *   @file: Fat32Utils.cpp
 *
 *   @date: Jul 12, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Utils.h"

using namespace kstd;
namespace filesystem {

bool Fat32Utils::fits_in_8_3(const kstd::string& filename) {
    if (filename.empty())
        return false;

    auto dot_pos = filename.rfind('.');

    // case1. name with no extension
    if (dot_pos == string::npos)
        return filename.length() <= 8;

    // case2. name with extension
    if (dot_pos > 8)
        return false;

    if ((filename.length() - dot_pos - 1) > 3)
        return false;

    return true;
}

/**
 * @brief   Generate valid 8_3 filename
 * @param   seq_num Number used if need to shorten the name, eg for seq_num 1: "longfilename.txt" -> "LONGFI~1.TXT"
 */
string Fat32Utils::make_8_3_filename(const string& filename, u8 seq_num) {
    string name, ext;
    make_8_3_filename(filename, name, ext, seq_num);
    return ext.empty() ? name : name + "." + ext;
}

/**
 * @brief   Generate valid 8_3 filename
 * @param   seq_num Number used if need to shorten the name, eg for seq_num 1: "longfilename" -> "LONGI~1"
 */
void Fat32Utils::make_8_3_filename(const kstd::string& filename, kstd::string& name, kstd::string& ext, u8 seq_num) {
    // split filename -> name:ext on '.', or use empty ext
    if (filename.rfind('.') == string::npos) {
        name = filename;
        ext = "";
    } else {
        split_key_value(filename, name, ext, '.');
    }

    // if name length > 8 characters, reduce it to have 8 characters including well known DOS "~num" ending
    if (name.length() > 8) {
        string seq = to_str(seq_num);
        name.resize(8 - seq.length() - 1);
        name += "~" + seq;
    }

    if (ext.length() > 3) {
        ext.resize(3);
    }

    // make the name and ext upper case
    name = to_upper_case(name);
    ext = to_upper_case(ext);
}

void Fat32Utils::make_8_3_space_padded_filename(const string& filename, string& name, string& ext)  {
    make_8_3_filename(filename, name, ext);

    // pad name with space up to 8 characters length
    for (u32 i = name.length(); i < 8; i++)
         name.push_back(' ');


    // pad ext with space up to 3 characters length
     for (u32 i = ext.length(); i < 3; i++)
         ext.push_back(' ');
}

} /* namespace filesystem */
