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


#include "Log.hpp"
#include "psc_format.hpp"
#include "LogViewSyslog.hpp"
//#ifdef SYSLOG

namespace psc {
namespace log {


LogViewSyslog::LogViewSyslog()
: LogViewFile::LogViewFile()
{
}

LogViewIterator
LogViewSyslog::begin()
{
    auto logViewSyslog = shared_from_this();
    auto logViewFileIterator = std::make_shared<LogViewSyslogIterator>(m_query, logViewSyslog);
    logViewFileIterator->inc();
    return LogViewIterator(logViewFileIterator);
}

LogViewIterator
LogViewSyslog::end()
{
    auto logViewFileIterator = std::make_shared<LogViewSyslogIterator>();
    return LogViewIterator(logViewFileIterator);
}

std::string
LogViewSyslog::getBasePath()
{
    return "/var/log";
}

LogViewEntry
LogViewSyslog::parse(const std::string& line)
{
    LogViewEntry logViewEntry;
    if (line.length() > 16) {
        // will most likely not match any syslog style...
        if (line.starts_with('[')) {    // variant used with pacman
            //[2024-11-16T06:05:23+0100] [ALPM] running '30-systemd-tmpfiles.hook'...
            auto timeEnd = line.find(']');
            if (timeEnd != line.npos) {
                std::string in{line.substr(1, timeEnd-1)};
                LogTime timestamp;
                timestamp.parse("%FT%T%z", in, false);
                logViewEntry.setLocalTime(timestamp);
                timeEnd +=2;
                auto groupStart = line.find('[', timeEnd);
                if (groupStart != line.npos) {
                    ++groupStart;
                    auto groupEnd = line.find(']', groupStart);
                    if (groupEnd != line.npos) {
                        logViewEntry.setLocation(line.substr(groupStart, groupEnd - groupStart));
                        groupEnd += 2;
                        logViewEntry.setMessage(line.substr(groupEnd));
                        logViewEntry.setLevel(Level::Info);
                    }
                }
            }
        }
        else {
            //May 11 10:40:48 scrooge disk-health-nurse[26783]: [ID 702911 user.error] m:SY-mon-full-500 c:H : partition health
            std::string date = line.substr(0, 7);
            std::string time = line.substr(7, 8);
            auto year = LogTime::now().format("%Y ");
            std::string dateTime(date + year + time);
            LogTime timestamp;
            timestamp.parse("%b %d %Y %T", dateTime, true);
            logViewEntry.setLocalTime(timestamp);
            size_t hostPos = line.find(' ', 16);
            if (hostPos != line.npos) {
                std::string host;
                host = line.substr(16, hostPos-16);
                size_t processPos = line.find(' ', ++hostPos);
                if (processPos != line.npos) {
                    std::string process;
                    process = line.substr(hostPos, processPos-hostPos);
                    std::string location;
                    location = host + " " + process;
                    ++processPos;   // skip space
                    if (line[processPos] == '[') {
                        ++processPos;
                    }
                    size_t idPos = line.find(']', processPos);
                    if (idPos != line.npos) {
                        std::string id;
                        id = line.substr(processPos, idPos-processPos);
                        size_t prioStart = id.find_last_of('.');
                        Level level = Level::Info;
                        if (prioStart != id.npos) {
                            ++prioStart;
                            auto sLevel = id.substr(prioStart);
                            level = Log::getLevel(sLevel);  // ?may vary with locale
                            location += " " + id;
                        }
                        ++idPos;
                        logViewEntry.setMessage(line.substr(idPos));
                        logViewEntry.setLocation(location);
                        logViewEntry.setLevel(level);
                    }
                }
            }
        }
    }
    return logViewEntry;
}

pLogViewSyslog
LogViewSyslog::create()
{
    return std::make_shared<LogViewSyslog>();
}


LogViewEntry
LogViewSyslogIterator::parse()
{
    std::string line;
    std::getline(m_stat, line);
    return m_logViewFile->parse(line);
}


} /* namespace log */
} /* namespace psc */
