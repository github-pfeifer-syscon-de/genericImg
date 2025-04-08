/*
 * Copyright (C) 2025 RPf <gpl3@pfeifer-syscon.de>
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

#include "KeyConfig.hpp"
#include "BinModel.hpp"
#include "DateUtils.hpp"

bool
test_keyfile()
{
    KeyConfig conf("test");
    conf.setString("grp", "key", "abc;def");
    std::vector<Glib::ustring> list = conf.getStringList("grp", "key");
    if (list.size() != 2
      ||list[0] != "abc"
      ||list[1] != "def") {
        std::cout << "test_keyfile expected 2 got " << list.size() << std::endl;
        size_t i = 0;
        auto print = [&i](const auto& v) {
            std::cout << "list[" << i++ << "] " << v << std::endl;
        };
        std::ranges::for_each(std::as_const(list), print);
        return false;
    }
    return true;
}

bool
init_test()
{
    Glib::Dispatcher dispatch;
    Glib::RefPtr<Gdk::Pixbuf> buf;
    BinModel binModel{dispatch, buf};
    if (!binModel.isZero()) {
        std::cout << "BinModel not zero!" << std::endl;
        return false;
    }
    return true;
}

bool
date_test()
{
    Glib::DateTime dt1 = psc::utl::DateUtils::parse("%F", "2025-04-08");
    if (dt1.get_year() != 2025
     || dt1.get_month() != 4
     || dt1.get_day_of_month() != 8) {
        std::cout << "DateTest1 expected 2025-04-08 got "
                  << dt1.get_year()
                  << "-" << dt1.get_month()
                  << "-" << dt1.get_day_of_month() << std::endl;
        return false;
    }
    Glib::DateTime dt2 = psc::utl::DateUtils::parse("%Y-%b-%d", "2025-Apr-08");
    if (dt2.get_year() != 2025
     || dt2.get_month() != 4
     || dt2.get_day_of_month() != 8) {
        std::cout << "DateTest2 expected 2025-04-08 got "
                  << dt2.get_year()
                  << "-" << dt2.get_month()
                  << "-" << dt2.get_day_of_month() << std::endl;
        return false;
    }
    if (!psc::utl::DateUtils::starts_with_ignore_case("LaLa", "lA")) {
        std::cout << "starts_with_ignore_case LaLa la "                  << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "en");      // make locale dependent, and make glib accept u8 const !!!
    Glib::init();
    if (!test_keyfile()) {
        return 1;
    }
    if (!init_test()) {
        return 2;
    }
    if (!date_test()) {
        return 3;
    }

    return 0;
}

