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

#pragma once

#include <glibmm.h>
#include <gdkmm.h>

class KeyConfig
{
public:
    KeyConfig(const char* configName);
    explicit KeyConfig(const KeyConfig& orig) = delete;
    virtual ~KeyConfig();

    Glib::ustring getString(const char* grp, const Glib::ustring& key, const char* def = "");
    void setString(const char* grp, const Glib::ustring& key, const Glib::ustring& value);
    int32_t getInteger(const char* grp, const Glib::ustring& key, const int32_t def = 0);
    void setInteger(const char* grp, const Glib::ustring& key, const int32_t value);
    bool getBoolean(const char* grp, const Glib::ustring& key, const bool def);
    void setBoolean(const char* grp, const Glib::ustring& key, const bool value);
    bool hasKey(const char* grp, const Glib::ustring& key);
    bool hasGrp(const char* grp);
    virtual void saveConfig();
    Glib::KeyFile* getConfig();
    std::string getConfigName();
    Gdk::RGBA getColor(const char* grp, const Glib::ustring& key);
    void setColor(const char* grp, const Glib::ustring& key, const Gdk::RGBA& rgba);
    Glib::ArrayHandle<Glib::ustring> getStringList(const char* grp, const Glib::ustring& key);
    Glib::ArrayHandle<int32_t> getIntegerList(const char* grp, const Glib::ustring& key);
    Glib::ArrayHandle<bool> getBoolList(const char* grp, const Glib::ustring& key);
    void setStringList(const char* grp, const Glib::ustring& key, Glib::ArrayHandle<Glib::ustring> strs);
    void setIntegerList(const char* grp, const Glib::ustring& key, Glib::ArrayHandle<int32_t> ints);
    void setBoolList(const char* grp, const Glib::ustring& key, Glib::ArrayHandle<bool> bools);

    static constexpr auto COLOR_BLACK = "rgb(0,0,0)";

protected:
    virtual void loadConfig();  // is called on creation

    Glib::ustring m_configName;
    Glib::KeyFile* m_config{nullptr};
private:

};

