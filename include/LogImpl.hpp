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

#pragma once

#include <source_location>

#include "Log.hpp"
#include "genericimg_config.h"


namespace psc::log {

#ifdef SYSDLOG
class SysdPlugin
: public LogPlugin
{
public:
    SysdPlugin(const char* prefix);
    explicit SysdPlugin(const SysdPlugin& orig) = delete;
    ~SysdPlugin() = default;

    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) override;
};
#endif

#ifdef SYSLOG
class SysPlugin
: public LogPlugin
{
public:
    SysPlugin(const char* prefix);
    explicit SysPlugin(const SysPlugin& orig) = delete;
    ~SysPlugin();

    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) override;
};
#endif

class ConsolePlugin
: public LogPlugin
{
public:
    ConsolePlugin(const char* prefix);
    explicit ConsolePlugin(const ConsolePlugin& orig) = delete;
    ~ConsolePlugin() = default;


    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) override;
};


} /* namespace psc::log */



