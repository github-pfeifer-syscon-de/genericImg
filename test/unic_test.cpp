/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2024 RPf 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "KeyConfig.hpp"
#include "StringUtils.hpp"
#include "BinModel.hpp"

// try to get use to the unicode "handling" / "island"
static bool
test_ustring()
{
    auto u8chr = u8"\t\r +0.314e+1";    // include white-space + sign + exponent
    double val = StringUtils::parseCDouble(StringUtils::u8str(u8chr));
    if (std::abs(val - 3.14) > 0.0000001) {
        std::cout << "expected value 3.14 got " << val << std::endl;
        return false;
    }
    auto dval{3.14};
    auto str = StringUtils::formatCDouble(dval);
    if (str != "3.14") {
        std::cout << "expected string \"3.14\" got " << str << std::endl;
        return false;
    }

    return true;
}

static bool
test_string_util()
{
    auto str = Glib::ustring("   abc   ");
    StringUtils::rtrim(str);
    if (str != "   abc") {
        std::cout << "rtrim expected \"abc\" got \"" << str << "\"" << std::endl;
        return false;
    }
    str = Glib::ustring("   abc   ");
    StringUtils::ltrim(str);
    if (str != "abc   ") {
        std::cout << "ltrim expected \"abc   \" got \"" << str << "\"" << std::endl;
        return false;
    }
    str = Glib::ustring("   abc   ");
    StringUtils::trim(str);
    if (str != "abc") {
        std::cout << "trim expected \"abc\" got \"" << str << "\"" << std::endl;
        return false;
    }
    str = Glib::ustring("   a  b  c");
    std::vector<Glib::ustring> parts;
    StringUtils::splitRepeat(str, ' ', parts);
    if (parts.size() != 3) {
        std::cout << "splitRepeat expected 3 got " << parts.size() << std::endl;
        return false;
    }
    if (parts[0] != "a") {
        std::cout << "splitRepeat expected \"a\" got " << parts[0] << std::endl;
        return false;
    }
    if (parts[1] != "b") {
        std::cout << "splitRepeat expected \"b\" got " << parts[1] << std::endl;
        return false;
    }
    if (parts[2] != "c") {
        std::cout << "splitRepeat expected \"c\" got " << parts[2] << std::endl;
        return false;
    }
    str = Glib::ustring("a,b,c");
    std::vector<Glib::ustring> parts2;
    StringUtils::split(str, ',', parts2);
    if (parts2.size() != 3) {
        std::cout << "split expected 3 got " << parts2.size() << std::endl;
        return false;
    }
    if (parts2[0] != "a") {
        std::cout << "split expected \"a\" got " << parts2[0] << std::endl;
        return false;
    }
    if (parts2[1] != "b") {
        std::cout << "split expected \"b\" got " << parts2[1] << std::endl;
        return false;
    }
    if (parts2[2] != "c") {
        std::cout << "split expected \"c\" got " << parts2[2] << std::endl;
        return false;
    }
    str = Glib::ustring("abc.def");
    bool ends = StringUtils::endsWith(str, ".def");
    if (!ends) {
        std::cout << std::boolalpha << "endsWith expected \"true\" got " << ends << std::endl;
        return false;
    }
    bool starts = StringUtils::startsWith(str, "abc.");
    if (!starts) {
        std::cout << std::boolalpha << "startsWith expected \"true\" got " << starts << std::endl;
        return false;
    }
    str = Glib::ustring("ab.c.d.ef");
    auto strrepl = StringUtils::replaceAll(str, ".e", "___");
    if (strrepl != "ab.c.d___f") {
        std::cout << "replaceAll expected \"ab.c.d___f\" got \"" << strrepl << "\"" << std::endl;
        return false;
    }
    str = Glib::ustring(StringUtils::u8str(u8"ABCD\u00C4"));
    auto strlow = StringUtils::lower(str, 1ul);
    if (strlow != "Abcd\u00E4") {
        std::cout << "lower expected \"Abcd\u00E4\" got \"" << strlow << "\"" << std::endl;
        return false;
    }
    auto tok = StringUtils::splitQuoted("This \"is a\"\" quoted\" str\"\"ing");
    if (tok.size() != 3
     || tok[0] != "This"
     || tok[1] != "is a\" quoted"
     || tok[2] != "str\"ing") {
        std::cout << "splitQuoted expected 3 got " << tok.size() << std::endl;
        for (size_t i = 0; i < tok.size(); ++i) {
            std::cout << "[" << i << "]=\"" << tok[i] << "\"" << std::endl;
        }
        return false;
    }
    auto file = Gio::File::create_for_path("/home/abc/x.yz.cpp");
    auto ext = StringUtils::getExtension(file);
    if (ext != "cpp") {
        std::cout << "getExtension expected cpp got \"" << ext << "\"" << std::endl;
        return false;
    }
    std::array<short, 8> arr{0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24};
    auto dmp = StringUtils::hexdump(arr.data(), arr.size());
    if (dmp != "0000 : 001d 001e 001f 0020 0021 0022 0023 0024     ... !\"#$\n") {
        std::cout << "Dump not as expected!" << std::endl << dmp;
        return false;
    }
    std::vector<std::string> vec{"abc","def","ghi"};
    std::function<std::string(const std::string& item)> lambda =
        [] (const std::string& item) -> auto
        {
            return item;
        };
    auto concated = StringUtils::concat(vec, std::string(","), lambda);
    if (concated != "abc,def,ghi") {
        std::cout << "Concated not as expected \"" << concated << "\"!" << std::endl;
        return false;
    }
    auto splitE = StringUtils::splitEach(concated, ',');
    if (splitE.size() != 3
      ||splitE[0] != "abc"
      ||splitE[1] != "def"
      ||splitE[2] != "ghi") {
        for (auto part : splitE) {
            std::cout << "part " << part << std::endl;
        }
        std::cout << "Split each not as expected " << splitE.size() << "!" << std::endl;
        return false;
    }
    auto splitC = StringUtils::splitConsec(std::string("abc   def ghi   "), ' ');
    if (splitC.size() != 3
      ||splitC[0] != "abc"
      ||splitC[1] != "def"
      ||splitC[2] != "ghi") {
        for (auto part : splitC) {
            std::cout << "part " << part << std::endl;
        }
        std::cout << "Split consec not as expected " << splitC.size() << "!" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Glib::init();
    if (!test_ustring()) {
        return 1;
    }
    if (!test_string_util()) {
        return 2;
    }

    return 0;
}

