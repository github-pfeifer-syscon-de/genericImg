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

#include <iostream>

#include "LogImpl.hpp"
#include "config.h"

#ifdef SYSDLOG
#include <systemd/sd-journal.h>
#endif
#ifdef SYSLOG
#include <syslog.h>
#endif


namespace psc {
namespace log {

ConsolePlugin::ConsolePlugin(const char* prefix)
: LogPlugin::LogPlugin(prefix)
{
}

void
ConsolePlugin::log(Level level
        , const Glib::ustring& msg
        , const std::source_location location)
{
    Glib::ustring time = Log::getTimestamp();
    if (level >= Level::Debug) {
        std::cout << location.function_name() << '\n';
    }
    std::cout << time
              << " " << Log::getLevel(level)
              << " " << location.file_name()
              << ":" << location.line()
              << " " << msg << std::endl;
}


#ifdef SYSDLOG
SysdPlugin::SysdPlugin(const char* prefix)
: LogPlugin::LogPlugin(prefix)
{
}

void
SysdPlugin::log(Level level, const Glib::ustring& msg, const std::source_location location)
{
    std::array<Glib::ustring, 5> ustr{
          Glib::ustring::sprintf("MESSAGE=%s", msg)
        , Glib::ustring::sprintf("CODE_FILE=%s", location.file_name())
        , Glib::ustring::sprintf("CODE_LINE=%d", location.line())
        , Glib::ustring::sprintf("CODE_FUNC=%s", location.function_name())
        , Glib::ustring::sprintf("PRIORITY=%d", static_cast<typename std::underlying_type<Level>::type>(level))
    };
    struct iovec iov[ustr.size()];
    for (uint32_t i = 0; i < ustr.size(); ++i) {
        // need const-cast as struct is defined as volatile
        iov[i].iov_base = const_cast<void*>(static_cast<const void*>(ustr[i].c_str()));
        iov[i].iov_len = ustr[i].length();
    }
    uint32_t iovcnt = ustr.size();
    int ret = sd_journal_sendv(iov, iovcnt);    // output as is
    if (ret != 0) {
        std::cout << "error send systemd log " << strerror(ret)
                  << " msg " << msg << std::endl;
    }
    //sd_journal_send is ... different
    //  (the prototype says "format" ... so its basically a syslog clone!)
}
#endif

#ifdef SYSLOG
SysPlugin::SysPlugin(const char* prefix)
: LogPlugin::LogPlugin(prefix)
{
    openlog(prefix, LOG_CONS|LOG_PID, LOG_USER);
}

SysPlugin::~SysPlugin()
{
    closelog();
}

void
SysPlugin::log(Level level, const Glib::ustring& msg, const std::source_location location)
{
    int32_t priority = static_cast<typename std::underlying_type<Level>::type>(level);
    syslog(priority, "%16s:%d %s", location.file_name(), location.line(), msg.c_str());
    // unused location.function_name()
}
#endif

} /* namespace log */
} /* namespace psc */

