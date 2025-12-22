/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf 
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
#include <glibmm.h>
#include <giomm.h>
#include <cstring>

#include "JsonObj.hpp"
#include "JsonHelper.hpp"

// the main intention is to check these functions with valgrind (who owns the returned data, just to be sure)

static bool
readTest()
{
    std::string json("{ \"root\" : \"abc\"}");
    auto data = Glib::ByteArray::create();
    data->append(reinterpret_cast<const guint8*>(json.c_str()), json.length());
    JsonHelper helper;
    helper.load_data(data);
    JsonObject* root = helper.get_root_object();
    auto value = json_object_get_string_member(root, "root");
    std::cout << "Read value " << "\"" << value << "\"" << std::endl;
    return !std::strcmp(value, "abc");
}

static bool
createTest()
{
    psc::json::JsonObj obj;
    obj.set("root", "abc");
    {
        auto inner = std::make_shared<psc::json::JsonObj>();
        inner->set("chld", "def");
        obj.set("inner", inner);
        std::cout << "after add inner" << std::endl;
    }
    auto str = obj.generate();
    std::cout << "Created " << str << std::endl;
    return str == "{\"root\":\"abc\",\"inner\":{\"chld\":\"def\"}}";
}

static bool
valueTest()
{
    psc::json::JsonValue sval("abc");
    std::cout << "valueTest str " << sval.getString() << std::endl;
    if (!sval.isString() || sval.getString() != "abc") {
        return false;
    }
    psc::json::JsonValue ival(4711l);
    std::cout << "valueTest int " << ival.getInt() << std::endl;
    if (!ival.isInt() || ival.getInt() != 4711l) {
        return false;
    }

    psc::json::JsonValue bval(true);
    std::cout << "valueTest bool " << std::boolalpha << bval.getBool() << std::endl;
    if (!bval.isBool() || bval.getBool() != true) {
        return false;
    }
    psc::json::JsonValue dval(123.456);
    std::cout << "valueTest double " << dval.getDouble() << std::endl;
    if (!dval.isDouble() || dval.getDouble() != 123.456) {
        return false;
    }
    auto obj = std::make_shared<psc::json::JsonObj>();
    obj->set("abc", 123l);
    psc::json::JsonValue oval(obj);
    std::cout << "valueTest obj" << std::endl;
    if (!oval.isObject() || oval.getObject()->getValue("abc")->getInt() != 123) {
        return false;
    }

    return true;
}

static bool
arrayTest()
{
    auto iarr = std::make_shared<psc::json::JsonArr>(8);
    iarr->add({123, 456, 789});
    psc::json::JsonValue aval(iarr);
    if (!aval.isArray()
      || aval.getArray()->getSize() != 3
      || aval.getArray()->get(2)->getInt() != 789) {
        std::cout << "arrayTest int expected 789 got " << aval.getArray()->get(2)->getInt() << std::endl;
        return false;
    }
    psc::json::JsonArr farr(8);
    farr.add({1.23, 4.56, 7.89});
    if (farr.getSize() != 3
     || farr[2]->getDouble() != 7.89) {
        std::cout << "arrayTest float expected 7.89 for " << farr[2]->getDouble() << std::endl;
        return false;
    }

    return true;
}



int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Gio::init();

    if (!readTest()) {
        return 1;
    }
    if (!createTest()) {
        return 2;
    }
    if (!valueTest()) {
        return 3;
    }
    if (!arrayTest()) {
        return 4;
    }
    return 0;
}
