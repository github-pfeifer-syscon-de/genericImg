/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2024 RPf <gpl3@pfeifer-syscon.de>
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
#include <glibmm.h>
#include <format>

#include "StringUtils.hpp"

// try to get use to the unicode "handling" / "island"
static bool
test_ustring() {

    auto u8chr = u8"\t\r +0.314e+1";    // include sign, white-space and exponent
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
    auto trims{StringUtils::u8str(u8" \r\t\u1680 abc \u000c ")};
    StringUtils::trim(trims);
    if (trims != "abc") {
        std::cout << "trim string \"abc\" got \"" << trims << "\"" << std::endl;
        return false;
    }
    auto split{StringUtils::u8str(u8"a\t\t\u1680b\tc")};
    std::vector<Glib::ustring> parts;
    StringUtils::split(split, '\t', parts);
    if (parts.size() != 4
     || parts[0] != Glib::ustring("a")
     || parts[1] != Glib::ustring("")
     || parts[2] != StringUtils::u8str(u8"\u1680b")
     || parts[3] != Glib::ustring("c")) {
        std::cout << "split expected 4 got \"" << parts.size() << "\"" << std::endl;
        return false;
    }
    std::vector<Glib::ustring> rparts;
    StringUtils::splitRepeat(split, '\t', rparts);
    if (rparts.size() != 3
     || rparts[0] != Glib::ustring("a")
     || rparts[1] != StringUtils::u8str(u8"\u1680b")
     || rparts[2] != Glib::ustring("c")) {
        std::cout << "splitRepeat expected 3 got \"" << parts.size() << "\"" << std::endl;
        return false;
    }
    auto repl{StringUtils::u8str(u8"a\t\u1680b\tc")};
    auto res = StringUtils::replaceAll(repl, "\t", "-*-");
    auto exp = StringUtils::u8str(u8"a-*-\u1680b-*-c");
    if (res != exp) {
        std::cout << "replace expected \"" << exp << "\" got \"" << res << "\"" << std::endl;
        return false;
    }
    if (!StringUtils::startsWith(repl, "a\t")) {
        std::cout << "startsWith false got true "<< std::endl;
        return false;
    }
    if (!StringUtils::endsWith(repl, "\tc")) {
        std::cout << "endsWith false got true "<< std::endl;
        return false;
    }

    return true;
}

static bool
test_format()
{
    Glib::ustring abc{"abc"};
    std::string std{"abc"};
    // this has all the fancy stuff ;)
    std::cout << std::format("Hello std::format |{0:>10}| |{1:>10}|", abc, std) << std::endl;
    // this has no formatting abilities :( as far as i see
    std::cout << Glib::ustring::compose("Hello Glib::compose |%1| |%2|", abc, std) << std::endl;
    double d{123.456};
    std::cout << std::format("{0:.0f}", d) << std::endl;

    return true;
}

/*
 *
 */
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Glib::init();
    if (!test_ustring()) {
        return 1;
    }
    if (!test_format()) {
        return 1;
    }

    return 0;
}

