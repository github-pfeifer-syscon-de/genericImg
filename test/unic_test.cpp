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

#include "StringUtils.hpp"

// try to get use to the unicode "handling" / "island"
static bool
test_ustring() {

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

    return 0;
}

