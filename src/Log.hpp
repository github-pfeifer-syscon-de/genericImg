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
#include <memory>

#include "config.h"

namespace psc {
namespace log {


enum class Type {
    Default,
    File,
    Systemd,
    Console,
    None
};

enum class Level
{
    // see gd_errors.h to align with kernel
      Severe = 0
    , Alert
    , Crit
    , Error
    , Warn
    , Notice
    , Info
    , Debug
};

constexpr std::initializer_list<Level> all_Levels = {
      Level::Severe
    , Level::Alert
    , Level::Crit
    , Level::Error
    , Level::Warn
    , Level::Notice
    , Level::Info
    , Level::Debug
};


class LogPlugin
{
public:
    LogPlugin(const char* prefix);
    explicit LogPlugin(const LogPlugin& orig) = delete;
    virtual ~LogPlugin() = default;
    virtual void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) = 0;
    virtual void close()
    {
    }
protected:
    Glib::ustring m_prefix;
};

class FilePlugin
: public LogPlugin
{
public:
    FilePlugin(const char* prefix);
    explicit FilePlugin(const FilePlugin& orig) = delete;
    ~FilePlugin();

    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) override;
    void setSizeLimit(goffset sizeLimit);
    goffset getSizeLimit();

    void close() override;
    void create();
    static constexpr auto DEFAULT_SIZELIMITI{102400ul};
    void createLogFile(const Glib::RefPtr<Gio::File>& file);

private:
    Glib::RefPtr<Gio::FileOutputStream> m_outstream;
    goffset m_sizeLimit;
};

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

#ifdef SYSDLOG
class SysPlugin
: public LogPlugin
{
public:
    SysPlugin(const char* prefix);
    explicit SysPlugin(const SysPlugin& orig) = delete;
    ~SysPlugin() = default;


    void log(Level level
            , const Glib::ustring& msg
            , const std::source_location location) override;
};
#endif


class Log
{
public:
    Log(const char* prefix, Type type = Type::Default);
    explicit Log(const Log& orig) = delete;
    virtual ~Log() = default;

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
    void log(Level level
            , std::function< Glib::ustring(void) >&& lambda
            , const std::source_location location = std::source_location::current());
    inline bool isLoggable(Level level)
    {
        return level <= m_level;
    }
    [[deprecated("see logAdd")]]
    static void logNow(Level level
            , const Glib::ustring& msg
	    , const std::source_location location = std::source_location::current());
    // convenience function, logs to previously created logger
    static void logAdd(Level level
            , const Glib::ustring& msg
	    , const std::source_location location = std::source_location::current());
    // build to work with autotools DEBUG
    static void logAdd(const Glib::ustring& msg
            , int debug = 0
	    , const std::source_location location = std::source_location::current());
    // allow lazy evaluation of source string
    static void logAdd(Level level
            , std::function< Glib::ustring(void) >&& lambda
	    , const std::source_location location = std::source_location::current());
    Level getLevel();
    void setLevel(Level level);
    std::shared_ptr<LogPlugin> getPlugin();
    static Glib::ustring getTimestamp();
    static Level getLevel(const Glib::ustring& level);
    // this is a convenience method to use a unified log
    //   for different parts of a application.
    // The parameters are only honored on first invocation
    static std::shared_ptr<Log> create(const char* prefix, Type type = Type::Default);
    static const char* getLevel(Level level);
    void close();
private:
    Level m_level;
    std::shared_ptr<LogPlugin> m_plugin;
    static std::shared_ptr<Log> m_log;
};


} /* namespace log */
} /* namespace psc */

