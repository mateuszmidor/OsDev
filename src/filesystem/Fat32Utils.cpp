/**
 *   @file: Fat32Utils.cpp
 *
 *   @date: Jul 12, 2017
 * @author: Mateusz Midor
 */

#include "Fat32Utils.h"

using namespace kstd;
namespace filesystem {

string Fat32Utils::make_8_3_filename(const string& filename) {
    string name, ext;

    // split filename -> name:ext on '.', or use empty ext
    if (filename.rfind('.') == string::npos) {
        name = filename;
        ext = "";
    } else {
        split_key_value(filename, name, ext, '.');
    }

    // reduce name length to 8 and extension length to 3, pad them with spaces to have exactly that length
    if (name.length() > 8) {
        name.resize(7);
        name.push_back('~');
    }

    if (ext.length() > 3) {
        ext.resize(3);
    }

    // make the name and ext upper case
    name = to_upper_case(name);
    ext = to_upper_case(ext);

    return ext.empty() ? name : name + "." + ext;
}

void Fat32Utils::make_8_3_space_padded_filename(const string& filename, string& name, string& ext)  {
    string name_8_3 = make_8_3_filename(filename);

    // split filename -> name:ext on '.', or use empty ext
    if (name_8_3.rfind('.') == string::npos) {
        name = name_8_3;
        ext = "";
    } else
        split_key_value(name_8_3, name, ext, '.');


    // pad name with space up to 8 characters length
    for (u32 i = name.length(); i < 8; i++)
         name.push_back(' ');


    // pad ext with space up to 3 characters length
     for (u32 i = ext.length(); i < 3; i++)
         ext.push_back(' ');
}

} /* namespace filesystem */
