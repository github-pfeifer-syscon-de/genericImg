/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include "psc_format.hpp"
#include "KeyConfig.hpp"

KeyConfig::KeyConfig(const char* configName)
: m_configName{configName}
, m_config{new Glib::KeyFile()}     // there should be a create ... but the doc is wrong on this
{
    loadConfig();
}

KeyConfig::~KeyConfig()
{
    if (m_config) {
        delete m_config;
    }
}

Glib::ustring
KeyConfig::getString(const char* grp, const Glib::ustring& key, const char* def)
{
    if (hasKey(grp, key)) {
        return m_config->get_string(grp, key);
    }
    return def;
}

void
KeyConfig::setString(const char* grp, const Glib::ustring& key, const Glib::ustring& value)
{
    return m_config->set_string(grp, key, value);
}

int32_t
KeyConfig::getInteger(const char* grp, const Glib::ustring& key, const int32_t def)
{
    if (hasKey(grp, key)) {
        return m_config->get_integer(grp, key);
    }
    return def;
}

void
KeyConfig::setInteger(const char* grp, const Glib::ustring& key, const int32_t value)
{
    m_config->set_integer(grp, key, value);
}

double
KeyConfig::getDouble(const char* grp, const Glib::ustring& key, const double def)
{
    if (hasKey(grp, key)) {
        return m_config->get_double(grp, key);
    }
    return def;
}

void
KeyConfig::setDouble(const char* grp, const Glib::ustring& key, const double value)
{
    m_config->set_double(grp, key, value);
}

bool
KeyConfig::getBoolean(const char* grp, const Glib::ustring& key, const bool def)
{
    if (hasKey(grp, key)) {
        return m_config->get_boolean(grp, key);
    }
    return def;
}

void
KeyConfig::setBoolean(const char* grp, const Glib::ustring& key, const bool value)
{
    m_config->set_boolean(grp, key, value);
}

bool
KeyConfig::hasKey(const char* grp, const Glib::ustring& key)
{
    return m_config->has_group(grp)
        && m_config->has_key(grp, key);
}

bool
KeyConfig::hasGrp(const char* grp)
{
    return m_config->has_group(grp);
}

std::string
KeyConfig::getConfigName()
{
    auto fullPath = Glib::canonicalize_filename(
        static_cast<std::string>(m_configName), Glib::get_user_config_dir());
    return fullPath;
}

Glib::KeyFile*
KeyConfig::getConfig()
{
    return m_config;
}

void
KeyConfig::loadConfig()
{
    try {
        m_config->load_from_file(getConfigName());
    }
    catch (const Glib::FileError& exc) {
        std::cerr << "Cound not read " << exc.what() << " config " << m_configName << " (it may not yet exist and will be created)." << std::endl;
    }
}

void
KeyConfig::saveConfig()
{
    if (m_config) {
        m_config->save_to_file(getConfigName());
    }
}

Gdk::RGBA
KeyConfig::getColor(const char* grp, const Glib::ustring& key)
{
    Gdk::RGBA black{COLOR_BLACK};    // default to simple black
    return getColor(grp, key, black);
}

Gdk::RGBA
KeyConfig::getColor(const char* grp, const Glib::ustring& key, const Gdk::RGBA& dflt)
{
    auto str = getString(grp, key);
    Gdk::RGBA rgba = dflt;
    if (str.length() == 17) {   // read back our custom variant
        rgba.set(str.substr(0, 13));
        gushort alpha = static_cast<gushort>(std::stoul(str.substr(13, 4), nullptr, 16));
        rgba.set_alpha_u(alpha);
    }
    else if (!str.empty()) {
        rgba.set(str);
    }
    return rgba;
}

std::string
KeyConfig::encodeColor(const Gdk::RGBA& rgba)
{
    std::string colorRgb;
    if (rgba.get_alpha() != 1.0) {
        // use custom format seems to be the only option to store 16bit precision  with alpha
        colorRgb = psc::fmt::format("#{:04x}{:04x}{:04x}{:04x}",
                                   rgba.get_red_u()
                                 , rgba.get_green_u()
                                 , rgba.get_blue_u()
                                 , rgba.get_alpha_u());
    }
    else {
        // use 16bit precision to_string may reduce this
        colorRgb = psc::fmt::format("#{:04x}{:04x}{:04x}",
                                   rgba.get_red_u()
                                 , rgba.get_green_u()
                                 , rgba.get_blue_u());

    }
    return colorRgb;
}

void
KeyConfig::setColor(const char* grp, const Glib::ustring& key, const Gdk::RGBA& rgba)
{
    std::string colorRgb{encodeColor(rgba)};
    setString(grp, key, colorRgb);
}

Glib::ArrayHandle<Glib::ustring>
KeyConfig::getStringList(const char* grp, const Glib::ustring& key)
{
    if (hasKey(grp, key)) {
        return m_config->get_string_list(grp, key);
    }
    return std::vector<Glib::ustring>();
}

Glib::ArrayHandle<int32_t>
KeyConfig::getIntegerList(const char* grp, const Glib::ustring& key)
{
    if (hasKey(grp, key)) {
        return m_config->get_integer_list(grp, key);
    }
    return std::vector<int32_t>();
}

Glib::ArrayHandle<bool>
KeyConfig::getBoolList(const char* grp, const Glib::ustring& key)
{
    if (hasKey(grp, key)) {
        return m_config->get_boolean_list(grp, key);
    }
    return std::vector<bool>();
}

void
KeyConfig::setStringList(const char* grp, const Glib::ustring& key, const Glib::ArrayHandle<Glib::ustring> strs)
{
    m_config->set_string_list(grp, key, strs);
}

void
KeyConfig::setIntegerList(const char* grp, const Glib::ustring& key, const Glib::ArrayHandle<int32_t> ints)
{
    m_config->set_integer_list(grp, key, ints);
}

void
KeyConfig::setBoolList(const char* grp, const Glib::ustring& key, const Glib::ArrayHandle<bool> bools)
{
    m_config->set_boolean_list(grp, key, bools);
}

Pango::FontDescription
KeyConfig::getFont(const char* grp, const Glib::ustring& key, const Glib::ustring& deflt)
{
    auto str = getString(grp, key);
    if (str.empty()) {
        str = deflt;
    }
    auto descr = Pango::FontDescription(str);
    return descr;
}

void KeyConfig::setFont(const char* grp, const Glib::ustring& key, const Pango::FontDescription& descr)
{
    Glib::ustring str = descr.to_string();
    setString(grp, key, str);
}

