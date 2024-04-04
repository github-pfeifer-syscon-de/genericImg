/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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
#include <giomm-2.4/giomm.h>
#include <source_location>


namespace psc {
namespace log {

enum class Level {
    Error,
    Warn,
    Info,
    Debug,
    Trace
};


class Log
{
public:
    Log(const char* prefix);
    explicit Log(const Log& orig) = delete;
    virtual ~Log();

    void error(const Glib::ustring& msg
            , const std::source_location location = std::source_location::current());
    void warn(const Glib::ustring& msg
            , const std::source_location location = std::source_location::current());
    void info(const Glib::ustring& msg
            , const std::source_location location = std::source_location::current());
    void debug(const Glib::ustring& msg
            , const std::source_location location = std::source_location::current());
    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location = std::source_location::current());
    Level getLevel();
    void setLevel(Level level);
    void setSizeLimit(goffset sizeLimit);
    goffset getSizeLimit();
protected:
    void close();
    void create();
    Glib::ustring getLevel(Level level);
private:
    Glib::ustring m_prefix;
    Level m_level;
    Glib::RefPtr<Gio::FileOutputStream> m_outstream;
    goffset m_sizeLimit{1024000ul};
};


} /* namespace log */
} /* namespace psc */

