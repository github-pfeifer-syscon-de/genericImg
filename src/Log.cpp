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

#include <ctime>
#include <iostream>

#include "config.h"
#include "Log.hpp"
#include "LogImpl.hpp"

namespace psc {
namespace log {

std::shared_ptr<Log> Log::m_log;

LogPlugin::LogPlugin(const char* prefix)
: m_prefix{prefix}
{
}

FilePlugin::FilePlugin(const char* prefix)
: LogPlugin::LogPlugin(prefix)
, m_sizeLimit{DEFAULT_SIZELIMITI}
{
}

FilePlugin::~FilePlugin()
{
    close();
}

void
FilePlugin::close()
{
    if (m_outstream) {
        m_outstream->close();
        m_outstream.clear();
    }
}


void
FilePlugin::create()
{
    auto logPath = Glib::canonicalize_filename("log", Glib::get_home_dir());
    Glib::RefPtr<Gio::File> fileLogPath = Gio::File::create_for_path(logPath);
    if (!fileLogPath->query_exists()) {
        fileLogPath->make_directory();
    }
    auto name = m_prefix + ".log";
    auto fullPath = Glib::canonicalize_filename(name.c_str(), logPath);
    auto file = Gio::File::create_for_path(fullPath);
    createLogFile(file);
}

void
FilePlugin::createLogFile(const Glib::RefPtr<Gio::File>& file)
{
	bool createStrm = true;
    if (file->query_exists()) {
        auto fileAttr = file->query_info(G_FILE_ATTRIBUTE_STANDARD_SIZE);
        if (fileAttr->get_size() > m_sizeLimit) {
            file->remove(); // trunc existing alternative rolling
        }
        else {
			createStrm = false;
        }
    }
    if (createStrm) {
        m_outstream = file->create_file(Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
    }
	else {
        m_outstream = file->append_to();
	}
}

void
FilePlugin::setSizeLimit(goffset sizeLimit)
{
    m_sizeLimit = sizeLimit;
}

goffset
FilePlugin::getSizeLimit()
{
    return m_sizeLimit;
}

void
FilePlugin::log(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    if (!m_outstream) {
        create();
    }
    Glib::ustring time = Log::getTimestamp();
    if (level >= Level::Debug) {
        Glib::ustring fn{location.function_name()};
        m_outstream->write("                                   " + fn + "\n");
    }
    Glib::ustring out = Glib::ustring::sprintf("%s %s %20s:%3d %s\n"
              , time
              , Log::getLevel(level)
              , location.file_name()
              , location.line()
              , msg) ;
    m_outstream->write(out);
}

static std::shared_ptr<LogPlugin>
defaultLogType(const char* prefix)
{
#   ifdef SYSDLOG
        return std::make_shared<SysdPlugin>(prefix);
#   else
#       ifdef SYSLOG
            return std::make_shared<SysPlugin>(prefix);
#       else
            return std::make_shared<FilePlugin>(prefix);
#       endif
#   endif
}

Log::Log(const std::shared_ptr<LogPlugin>& plugin)
: m_level{Level::Info}
, m_plugin{plugin}
{
}

Log::~Log()
{
    close();
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

std::shared_ptr<LogPlugin>
Log::getPlugin()
{
    return m_plugin;
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

void
Log::log(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    if (m_plugin && isLoggable(level)) {
        m_plugin->log(level, msg, location);
    }
}

void
Log::log(Level level
        , std::function< Glib::ustring(void) >&& lambda
        , const std::source_location location)
{
    if (m_plugin && isLoggable(level)) {
        m_plugin->log(level, lambda(), location);
    }
}

void
Log::logAdd(const Glib::ustring& msg
        , int debug
        , const std::source_location location
        )
{
    if (debug > 0) {
        logAdd(Level::Debug, msg, location);
    }
}

void
Log::logNow(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    logAdd(level, msg, location);
}

void
Log::logAdd(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    if (m_log) {
        if (m_log->isLoggable(level)) {
            m_log->log(level, msg, location);
        }
    }
    else {
        std::cout << getTimestamp()
                  << " " << getLevel(level)
                  << " " << msg << std::endl;
    }
}

void
Log::logAdd(Level level
        , std::function< Glib::ustring(void) >&& lambda
	    , const std::source_location location)
{
	if (m_log) {
        if (m_log->isLoggable(level)) {
            m_log->log(level, lambda(), location);
    	}
	}
    else {
        std::cout << getTimestamp()
                  << " " << getLevel(level)
				  << " " << lambda() << std::endl;
    }
}


Glib::ustring
Log::getTimestamp()
{
    auto dateTime = Glib::DateTime::create_now_local();
    Glib::ustring time;
    if (dateTime) {
        time = dateTime.format("%F %T.%f");
        time = time.substr(0, time.length() - 3);   // cut us
    }
//  else {
//       time_t rawtime;
//       std::time(&rawtime);
//       struct tm* timeinfo = std::localtime(&rawtime);
//       char buffer[80];
//       std::strftime(buffer, sizeof(buffer), "%F %T", timeinfo);
//       time = buffer;
//   }
    return time;
}

const char*
Log::getLevel(Level level)
{
    switch (level) {
    case Level::Severe:
        return "Sev";
    case Level::Alert:
        return "Alt";
    case Level::Crit:
        return "Cri";
    case Level::Error:
        return "Err";
    case Level::Warn:
        return "Wrn";
    case Level::Notice:
        return "Not";
    case Level::Info:
        return "Inf";
    case Level::Debug:
        return "Deb";
    }
    return "???";
}

const char*
Log::getLevelFull(Level level)
{
    switch (level) {
    case Level::Severe:
        return "Severe";
    case Level::Alert:
        return "Alert";
    case Level::Crit:
        return "Critical";
    case Level::Error:
        return "Error";
    case Level::Warn:
        return "Warning";
    case Level::Notice:
        return "Notice";
    case Level::Info:
        return "Info";
    case Level::Debug:
        return "Debug";
    }
    return "???";
}

Level
Log::getLevel(const Glib::ustring& level)
{
    if (level.size() > 0) {
        switch (level.lowercase().at(0)) {
        case 's':
            return Level::Severe;
        case 'a':
            return Level::Alert;
        case 'c':
            return Level::Crit;
        case 'e':
            return Level::Error;
        case 'w':
            return Level::Warn;
        case 'n':
            return Level::Notice;
        case 'i':
            return Level::Info;
        case 'd':
            return Level::Debug;
        }
    }
    return Level::Info; // presume info if anything goes wrong
}

std::shared_ptr<Log>
Log::create(const char* prefix, Type type)
{
    if (!m_log) {
        std::shared_ptr<LogPlugin> plugin;
        switch (type) {
        case Type::Default:
            plugin = defaultLogType(prefix);
            break;
        case Type::File:
            plugin = std::make_shared<FilePlugin>(prefix);
            break;
        case Type::Console:
            plugin = std::make_shared<ConsolePlugin>(prefix);
            break;
        case Type::None:
            plugin.reset();
            break;
        default:
            std::cerr << "No logging set!" << std::endl;
            break;
        }
        m_log = std::make_shared<Log>(plugin);
    }
    return m_log;
}

std::shared_ptr<Log>
Log::getGlobalLog()
{
    return m_log;
}

void
Log::close()
{
    if (m_plugin) {
        m_plugin->close();
    }
}

} /* namespace log */
} /* namespace psc */


