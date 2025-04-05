/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4 -*-  */
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

#pragma once

#include <glibmm.h>
#include <gio/gio.h>
#include <memory>
#include <type_traits>
#include <json-glib/json-glib.h>

namespace psc::json {

class JsonObj;
class JsonArr;
class JsonValue;

using PtrJsonValue = std::shared_ptr<JsonValue>;
using PtrJsonObj = std::shared_ptr<JsonObj>;
using PtrJsonArr = std::shared_ptr<JsonArr>;


class JsonValue
{
public:
    explicit JsonValue(Glib::UStringView value);
    explicit JsonValue(const char* value);
    explicit JsonValue(gint64 value);
    explicit JsonValue(const double& value);
    explicit JsonValue(bool value);
    explicit JsonValue(const PtrJsonObj& obj);
    explicit JsonValue(const PtrJsonArr& arr);
    explicit JsonValue(JsonNode* node);
    explicit JsonValue(const JsonValue& orig) = delete;
    virtual ~JsonValue();
    JsonNode* getNode();
    bool isInt();
    gint64 getInt();
    bool isString();
    Glib::ustring getString();
    bool isBool();
    bool getBool();
    bool isDouble();
    double getDouble();
    bool isObject();
    PtrJsonObj getObject();
    bool isArray();
    PtrJsonArr getArray();
    Glib::ustring generate(uint32_t indent = 0);
protected:
private:
    JsonNode* m_node;
};


class JsonObj
{
public:
    JsonObj();
    JsonObj(JsonObject* m_jsonObj);
    explicit JsonObj(const JsonObj& orig) = delete;
    virtual ~JsonObj();

    void set(Glib::UStringView key, Glib::UStringView value);
    void set(Glib::UStringView key, gint64 val);
    void set(Glib::UStringView key, const PtrJsonValue& value);
    void set(Glib::UStringView key, const PtrJsonObj& obj);
    PtrJsonObj createObj(Glib::UStringView key);
    void set(Glib::UStringView key, const std::shared_ptr<JsonArr>& arr);
    PtrJsonArr createArr(Glib::UStringView key, guint size = 0);
    PtrJsonValue getValue(Glib::UStringView key);
    Glib::ustring generate(uint32_t indent = 0);
    JsonObject* getObj();
private:
    JsonObject* m_jsonObj;
};


class JsonArr
{
public:
    JsonArr(guint size = 0);
    JsonArr(JsonArray* arr);
    explicit JsonArr(const JsonArr& orig) = delete;
    virtual ~JsonArr();

    void add(Glib::UStringView value);
    void add(const std::shared_ptr<JsonValue>& value);
    void add(const std::shared_ptr<JsonObj>& obj);
    template< typename T
            , typename = typename std::enable_if<std::is_integral<T>::value, T>::type
            , size_t N
            //, typename = typename std::enable_if< 1 < N >::type     // avoid activation for (no) sized init 1 < N is N >= 2
            >
    inline void add(const T (&arr) [N])
    {
        for (size_t i = 0; i < N; ++i) {
            json_array_add_int_element(m_jsonArr, arr[i]);
        }
    }
//    template< typename T
//            , typename = typename std::enable_if<std::is_floating_point<T>::value, T>::type
//            , size_t N
//            //, typename = typename std::enable_if< 1 < N >::type     // avoid activation for (no) sized init 1 < N is N >= 2
//            >
//    inline void add(const T (&arr) [N])
//    {
//        for (size_t i = 0; i < N; ++i) {
//            json_array_add_double_element(m_jsonArr, arr[i]);
//        }
//    }

    Glib::ustring generate(uint32_t indent = 0);
    PtrJsonValue get(guint idx);
    guint getSize();
    JsonArray* getArray();
private:
    JsonArray* m_jsonArr;
};

} /* namespace psc::json */
