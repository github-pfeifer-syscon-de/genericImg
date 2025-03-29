/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include "JsonObj.hpp"

namespace psc {
namespace json {

JsonValue::JsonValue(Glib::UStringView value)
{
    m_node = json_node_new(JSON_NODE_VALUE);
    m_node = json_node_init_string(m_node, value.c_str());
}

// important as bool catches this too otherwise :(
JsonValue::JsonValue(const char* value)
{
    m_node = json_node_new(JSON_NODE_VALUE);
    m_node = json_node_init_string(m_node, value);
}

JsonValue::JsonValue(gint64 value)
{
    m_node = json_node_new(JSON_NODE_VALUE);
    json_node_init_int(m_node, value);
}

JsonValue::JsonValue(const double& value)
{
    m_node = json_node_new(JSON_NODE_VALUE);
    json_node_init_double(m_node, value);
}

JsonValue::JsonValue(bool value)
{
    m_node = json_node_new(JSON_NODE_VALUE);
    json_node_init_boolean(m_node, value);
}

JsonValue::JsonValue(const std::shared_ptr<JsonObj>& obj)
{
    m_node = json_node_new(JSON_NODE_OBJECT);
    json_node_init_object(m_node, obj->getObj());
}

JsonValue::JsonValue(const std::shared_ptr<JsonArr>& arr)
{
    m_node = json_node_new(JSON_NODE_ARRAY);
    json_node_init_array(m_node, arr->getArray());
}


JsonValue::JsonValue(JsonNode* node)
: m_node{node}
{
    json_node_ref(m_node);        // usually retured values are commented owned by instance, so have to ref theses
}

JsonValue::~JsonValue()
{
    if (m_node) {
        json_node_unref(m_node);
        m_node = nullptr;
    }
}

JsonNode*
JsonValue::getNode()
{
    return m_node;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"   // switches are incomplete by intention

bool
JsonValue::isInt()
{
    auto nodeType = json_node_get_node_type(m_node);
    switch(nodeType) {
    case JSON_NODE_VALUE:
        auto valType = json_node_get_value_type(m_node);
        switch (valType) {
        case G_TYPE_INT:
        case G_TYPE_INT64:
            return true;
        }
        break;
    }
    return false;
}

gint64
JsonValue::getInt()
{
    if (isInt()) {
        return json_node_get_int(m_node);
    }
    return 0l;
}



bool
JsonValue::isString()
{
    auto nodeType = json_node_get_node_type(m_node);
    switch(nodeType) {
    case JSON_NODE_VALUE:
        auto valType = json_node_get_value_type(m_node);
        // to debug type issues use:  " name " << g_type_name(valType)
        return g_type_is_a(valType, G_TYPE_STRING);
    }
    return false;
}

Glib::ustring
JsonValue::getString()
{
    if (isString()) {
        const char *str = json_node_get_string(m_node);
        if (str) {
            return Glib::ustring(str);
        }
    }
    return "";
}

bool
JsonValue::isBool()
{
    auto nodeType = json_node_get_node_type(m_node);
    switch(nodeType) {
    case JSON_NODE_VALUE:
        auto valType = json_node_get_value_type(m_node);
        return valType == G_TYPE_BOOLEAN;
    }
    return false;
}

bool
JsonValue::getBool()
{
    if (isBool()) {
        return json_node_get_boolean(m_node);
    }
    return false;
}

bool
JsonValue::isDouble()
{
    auto nodeType = json_node_get_node_type(m_node);
    switch(nodeType) {
    case JSON_NODE_VALUE:
        auto valType = json_node_get_value_type(m_node);
        return valType== G_TYPE_DOUBLE;
    }
    return false;
}

double
JsonValue::getDouble()
{
    if (isDouble()) {
        return json_node_get_double(m_node);
    }
    return 0.0;
}

bool
JsonValue::isObject()
{
    auto nodeType = json_node_get_node_type(m_node);
    return nodeType == JSON_NODE_OBJECT;
}

std::shared_ptr<JsonObj>
JsonValue::getObject()
{
    std::shared_ptr<JsonObj> obj;
    if (isObject()) {
        auto o = json_node_get_object(m_node);
        if (o) {
            obj = std::make_shared<JsonObj>(o);
        }
    }
    return obj;
}

bool
JsonValue::isArray()
{
    auto nodeType = json_node_get_node_type(m_node);
    return nodeType == JSON_NODE_ARRAY;
}

std::shared_ptr<JsonArr>
JsonValue::getArray()
{
    std::shared_ptr<JsonArr> arr;
    if (isArray())  {
        auto a = json_node_get_array(m_node);
        if (a) {
            arr = std::make_shared<JsonArr>(a);
        }
    }
    return arr;
}


#pragma GCC diagnostic pop


//    case JSON_NODE_ARRAY:
//        auto jsonArr = json_node_dup_array(node);
//        break;
//    case JSON_NODE_NULL:
//        break;
//    case JSON_NODE_OBJECT:
//        auto jsonObj = json_node_dup_object(node);
//        break;
//    }


JsonObj::JsonObj()
: m_jsonObj{json_object_new()}
{

}

JsonObj::JsonObj(JsonObject* jsonObj)
: m_jsonObj{jsonObj}
{
    // as instances come with owned by
    json_object_ref(m_jsonObj);
}

JsonObj::~JsonObj()
{
    if (m_jsonObj) {
        json_object_unref(m_jsonObj);
        m_jsonObj = nullptr;
    }
}

void
JsonObj::set(Glib::UStringView key, Glib::UStringView value)
{
    json_object_set_string_member(m_jsonObj, key.c_str(), value.c_str());
}

void
JsonObj::set(Glib::UStringView key, gint64 val)
{
    json_object_set_int_member(m_jsonObj, key.c_str(), val);
}


void
JsonObj::set(Glib::UStringView key, const std::shared_ptr<JsonValue>& value)
{
    json_object_set_member(m_jsonObj, key.c_str(), value->getNode());
}

void
JsonObj::set(Glib::UStringView key, const std::shared_ptr<JsonObj>& value)
{
    // need to ref to keep refcount. as set_object_member "takes" the reference
    auto refObj = json_object_ref(value->getObj());
    json_object_set_object_member(m_jsonObj, key.c_str(), refObj);
}

std::shared_ptr<JsonObj>
JsonObj::createObj(Glib::UStringView key)
{
    auto obj = std::make_shared<JsonObj>();
    set(key.c_str(), obj);
    return obj;
}

void
JsonObj::set(Glib::UStringView key, const std::shared_ptr<JsonArr>& arr)
{
    // need to ref to keep refcount. as set_array_member "takes" the reference
    auto refArr = json_array_ref(arr->getArray());
    json_object_set_array_member(m_jsonObj, key.c_str(), refArr);
}

std::shared_ptr<JsonArr>
JsonObj::createArr(Glib::UStringView key, guint size)
{
    auto arr = std::make_shared<JsonArr>(size);
    set(key.c_str(), arr);
    return arr;
}

std::shared_ptr<JsonValue>
JsonObj::getValue(Glib::UStringView key)
{
    std::shared_ptr<JsonValue> val;
    auto node = json_object_get_member(m_jsonObj, key.c_str());
    if (node) {
        val = std::make_shared<JsonValue>(node);
    }
    return val;
}

Glib::ustring
JsonObj::generate(uint32_t indent)
{
    g_autoptr(JsonGenerator) jsonGen = json_generator_new();
    if (indent > 0) {
        json_generator_set_indent(jsonGen, indent);
        json_generator_set_indent_char(jsonGen, ' ');
        json_generator_set_pretty(jsonGen, true);
    }
    g_autoptr(JsonNode) node = json_node_new(JSON_NODE_OBJECT);
    json_node_init_object(node, m_jsonObj);
    json_generator_set_root(jsonGen, node);
    g_autofree GString* gstr = g_string_sized_new(256);
    json_generator_to_gstring(jsonGen, gstr);
    return Glib::ustring(gstr->str);
}

JsonObject*
JsonObj::getObj()
{
    return m_jsonObj;
}

JsonArr::JsonArr(guint size)
: m_jsonArr{json_array_sized_new(size)}
{

}

JsonArr::JsonArr(JsonArray* arr)
: m_jsonArr{arr}
{
    json_array_ref(m_jsonArr);
}

JsonArr::~JsonArr()
{
    if (m_jsonArr) {
        json_array_unref(m_jsonArr);
        m_jsonArr = nullptr;
    }
}


void JsonArr::add(Glib::UStringView value)
{
    json_array_add_string_element(m_jsonArr, value.c_str());
}

void JsonArr::add(const std::shared_ptr<JsonValue>& value)
{
    json_array_add_element(m_jsonArr, value->getNode());
}

void JsonArr::add(const std::shared_ptr<JsonObj>& obj)
{
    json_array_add_object_element(m_jsonArr, obj->getObj());
}

std::shared_ptr<JsonValue>
JsonArr::get(guint idx)
{
    auto node = json_array_get_element(m_jsonArr, idx);
    return std::make_shared<JsonValue>(node);
}

guint
JsonArr::getSize()
{
    return json_array_get_length(m_jsonArr);
}

JsonArray*
JsonArr::getArray()
{
    return m_jsonArr;
}

Glib::ustring
JsonArr::generate(uint32_t indent)
{
    g_autoptr(JsonGenerator) jsonGen = json_generator_new();
    if (indent > 0) {
        json_generator_set_indent(jsonGen, indent);
        json_generator_set_indent_char(jsonGen, ' ');
        json_generator_set_pretty(jsonGen, true);
    }
    g_autoptr(JsonNode) node = json_node_new(JSON_NODE_ARRAY);
    json_node_init_array(node, m_jsonArr);
    json_generator_set_root(jsonGen, node);
    g_autofree GString* gstr = g_string_sized_new(256);
    json_generator_to_gstring(jsonGen, gstr);
    return Glib::ustring(gstr->str);
}


} /* namespace json */
} /* namespace psc */