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

#include <ctime>

#include "Log.hpp"


namespace psc {
namespace log {


Log::Log(const char* prefix)
: m_prefix{prefix}
, m_level{Level::Info}
{
}

Log::~Log()
{
    close();
}

void Log::close()
{
    if (m_outstream) {
        m_outstream->close();
        m_outstream.clear();
    }
}

void
Log::create()
{
    auto logPath = Glib::canonicalize_filename("log", Glib::get_home_dir());
    auto name = m_prefix + ".log";
    auto fullPath = Glib::canonicalize_filename(name.c_str(), logPath);
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(fullPath);
    if (file->query_exists()) {
        auto fileAttr = file->query_info(G_FILE_ATTRIBUTE_STANDARD_SIZE);
        if (fileAttr->get_size() > m_sizeLimit) {
            file->remove();
            m_outstream = file->create_file(Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
        }
        else {
            m_outstream = file->append_to();
        }
    }
    else {
        // trunc existing alterantive rolling
        m_outstream = file->create_file(Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
    }
}

Level
Log::getLevel()
{
    return m_level;
}

void
Log::setLevel(Level level)
{
    m_level = level;
}

void
Log::setSizeLimit(goffset sizeLimit)
{
    m_sizeLimit = sizeLimit;
}

goffset
Log::getSizeLimit()
{
    return m_sizeLimit;
}

void
Log::error(const Glib::ustring& msg
         , const std::source_location location)
{
    log(Level::Error, msg, location);
}

void
Log::warn(const Glib::ustring& msg
         , const std::source_location location)
{
    log(Level::Warn, msg, location);
}

void
Log::info(const Glib::ustring& msg
         , const std::source_location location)
{
    log(Level::Info, msg, location);
}

void
Log::debug(const Glib::ustring& msg
         , const std::source_location location)
{
    log(Level::Debug, msg, location);
}

Glib::ustring
Log::getLevel(Level level)
{
    Glib::ustring slevel;
    switch (level) {
    case Level::Error:
        slevel = "Err";
        break;
    case Level::Warn:
        slevel = "Wrn";
        break;
    case Level::Info:
        slevel = "Inf";
        break;
    case Level::Debug:
        slevel = "Deb";
        break;
    case Level::Trace:
        slevel = "Trc";
        break;
    }
    return slevel;
}

void
Log::log(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    if (level <= m_level) {
        if (!m_outstream) {
            create();
        }
        auto dateTime = Glib::DateTime::create_now_local();
        Glib::ustring time;
        if (dateTime) {
            time = dateTime.format("%F %T.%f");
            time = time.substr(0, time.length() - 3);   // cut us
        }
//        else {
//            time_t rawtime;
//            std::time(&rawtime);
//            struct tm* timeinfo = std::localtime(&rawtime);
//            char buffer[80];
//            std::strftime(buffer, sizeof(buffer), "%F %T", timeinfo);
//            time = buffer;
//        }
        if (level >= Level::Debug) {
            Glib::ustring fn{location.function_name()};
            m_outstream->write(fn + "\n");
        }
        Glib::ustring out = Glib::ustring::sprintf("%s %s %20s:%3d %s\n"
                  , time
                  , getLevel(level)
                  , location.file_name()
                  , location.line()
                  , msg) ;
        m_outstream->write(out);
    }
}



} /* namespace log */
} /* namespace psc */


