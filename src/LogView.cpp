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

#include "config.h"
#include "LogView.hpp"

#ifdef SYSDLOG
#include "LogViewSysd.hpp"
#endif
#include "LogViewSyslog.hpp"
#include "DateUtils.hpp"



namespace psc {
namespace log {

LogViewException::LogViewException(const std::string& error)
: std::exception()
, swhat(error)
{
}

const char *
LogViewException::what() const noexcept
{
    return swhat.c_str();
}

LogDays::LogDays(const LogTime& logTime)
{
#ifdef USE_STD_CHRONO
    m_localDays = std::chrono::floor<std::chrono::days>(logTime.getLocalTime());
#else
    auto timestamp = logTime.getLocalTime();
    auto year = static_cast<Glib::Date::Year>(timestamp.get_year());
    auto month = static_cast<Glib::Date::Month>(timestamp.get_month());
    auto day = static_cast<Glib::Date::Day>(timestamp.get_day_of_month());
    LocalDays days{day, month, year};
    m_localDays = days;
#endif
}


Glib::ustring
LogDays::format(const char* fmt) const
{
#ifdef USE_STD_CHRONO
    std::string fmts = std::string{"{:"} + fmt + "}";
    return std::vformat(fmts, std::make_format_args(m_localDays));
#else
    return m_localDays.format_string(fmt);
#endif
}

void
LogDays::parseIsoDate(const std::string& date)
{
#ifdef USE_STD_CHRONO
    LocalTime timestamp;
    std::istringstream isoDay{date};
    isoDay >> std::chrono::parse("%F", timestamp);
    m_localDays = std::chrono::floor<std::chrono::days>(timestamp);
#else
    LocalTime timestamp = Glib::DateTime::create_from_iso8601(date + "T00:00:00", Glib::TimeZone::create_local());
    //std::cout << "LogDays::parseIsoDate"
    //          << " date " << date
    //          << " timestamp " << timestamp.format_iso8601() << std::endl;
    LogTime logTime{timestamp};
    LogDays logDays{logTime};
    m_localDays = logDays.m_localDays;
#endif
}

int
LogDays::compare(const LogDays& other) const
{
#ifdef USE_STD_CHRONO
    if (m_localDays < other.m_localDays) {
        return -1;
    }
    else if (m_localDays > other.m_localDays) {
        return 1;
    }
    return 0;
#else
    return m_localDays.compare(other.m_localDays);
#endif
}

bool
LogDays::operator<(const LogDays& other) const
{
    return compare(other) < 0;
}

bool
LogDays::operator>(const LogDays& other) const
{
    return compare(other) > 0;
}

bool
LogDays::operator==(const LogDays& other) const
{
    return compare(other) == 0;
}

bool
LogDays::isValid() const
{
#ifdef USE_STD_CHRONO
    auto sec = std::chrono::floor<std::chrono::seconds>(m_localDays);
    return sec.time_since_epoch().count() > 0;
#else
    return m_localDays.valid();
#endif
}

LogTime::LogTime(const LocalTime& localTime)
: m_localTime{localTime}
{
}

Glib::ustring
LogTime::format(const char* fmt) const
{
#ifdef USE_STD_CHRONO
    auto fmts = std::string{"{:"} + fmt + "}";
    return std::vformat(fmts, std::make_format_args(m_localTime));
#else
    return m_localTime.format(fmt);
#endif
}

void
LogTime::parseIsoDateTime(const std::string& dateTime)
{
#ifdef USE_STD_CHRONO
    std::istringstream in{dateTime};
    in >> std::chrono::parse("%F %T", m_localTime);  // iso-date and h:m:s
#else
    LocalTime timestamp = Glib::DateTime::create_from_iso8601(dateTime, Glib::TimeZone::create_local());
    m_localTime = timestamp;
#endif
}

LogDays
LogTime::toDays() const
{
#ifdef USE_STD_CHRONO
    return LogDays{m_localTime};
#else
    return LogDays{m_localTime};
#endif
}

bool
LogTime::isValid() const
{
#ifdef USE_STD_CHRONO
    auto sec = std::chrono::floor<std::chrono::seconds>(m_localTime);
    return sec.time_since_epoch().count() > 0;
#else
    return m_localTime.operator bool();
#endif
}


LocalTime
LogTime::getLocalTime() const
{
    return m_localTime;
}

int
LogTime::compare(const LogTime& other) const
{
#ifdef USE_STD_CHRONO
    if (m_localTime < other.m_localTime) {
        return -1;
    }
    else if (m_localTime > other.m_localTime) {
        return 1;
    }
    return 0;
#else
    return m_localTime.compare(other.m_localTime);
#endif
}

void
LogTime::parse(const char* fmt, const std::string& val, bool isLocal)
{
#ifdef USE_STD_CHRONO
    std::istringstream in{val};
    if (isLocal) {
        in >> std::chrono::parse(fmt, m_localTime);  //  %z is missing
    }
    else {
        std::chrono::time_point<std::chrono::system_clock> timestamp;
        in >> std::chrono::parse(fmt, timestamp);
        const auto tz = std::chrono::current_zone();
        auto local = tz->to_local(timestamp);
        m_localTime = std::chrono::time_point_cast<LogTimePrecision>(local);
    }
#else
    auto dateTime = psc::utl::DateUtils::parse(fmt, val, isLocal ? Glib::TimeZone::create_local() : Glib::TimeZone::create_utc());
    m_localTime = dateTime;
#endif
}

LogTime
LogTime::now()
{
#ifdef USE_STD_CHRONO
    auto now = std::chrono::system_clock::now();    // as a bootId ~equivalent use current day
    const auto tz = std::chrono::current_zone();
    auto local = tz->to_local(now);
    auto localPrec = std::chrono::time_point_cast<LogTimePrecision>(local);
    return LogTime{localPrec};
#else
    auto local = Glib::DateTime::create_now_local();
    return LogTime{local};
#endif
}

LogTime
LogTime::max()
{
#ifdef USE_STD_CHRONO
    return LogTime{LocalTime::max()};
#else
    return LogTime{LocalTime::create(Glib::TimeZone::create_local(), 9999, 12, 31, 0, 0, 0.0)};
#endif

}

LogTime
LogTime::min()
{
#ifdef USE_STD_CHRONO
    return LogTime{LocalTime::min()};
#else
    return LogTime{LocalTime::create(Glib::TimeZone::create_local(), 0001, 01, 01, 0, 0, 0.0)};
#endif
}

LogTime
LogTime::create_usec(uint64_t usec)
{
#   ifdef USE_STD_CHRONO
    auto epoch = std::chrono::system_clock::from_time_t(0); // we don't know what time_t counts, but 0 will always be 1970 UTC
    std::chrono::time_point<std::chrono::system_clock> clk;
    if (usec > 0) {
        auto dt = std::chrono::microseconds(usec);
        clk = epoch + dt;
    }
    else {
        clk = epoch;
    }
    const auto tz = std::chrono::current_zone();
    auto local = tz->to_local(clk);
    return LogTime{std::chrono::time_point_cast<LogTimePrecision>(local)};
#   else
    LocalTime time = Glib::DateTime::create_now_utc(usec/USEC);
    time.add_seconds(static_cast<double>(usec % USEC) / static_cast<double>(USEC));
    auto logTime = LogTime{time.to_local()};
    return logTime;
#   endif
}

std::ostream&
operator<< (std::ostream& os, const LogTime& logTime)
{
    os << logTime.format("%F %T");
    return os;
}

pLogView
LogView::create()
{
#   ifdef SYSDLOG
    return std::make_shared<LogViewSysd>();
    #else
#       ifdef SYSLOG
            return LogViewSyslog::create();
#       else
            return LogViewFile::create();
#       endif
#   endif
}

LogViewEntry::LogViewEntry()
: m_level{Level::Severe}
{
}

LogViewEntry::LogViewEntry(
          const std::string& message
        , const LogTime& logTime
        , const std::string& bootId
        , const std::string& location
        , Level level)
: m_message{message}
, m_logTime{logTime}
, m_bootId{bootId}
, m_location{location}
, m_level{level}
{
}

const std::string&
LogViewEntry::getMessage() const
{
    return m_message;
}

void
LogViewEntry::setMessage(const std::string& message)
{
    m_message = message;
}


const LogTime&
LogViewEntry::getLocalTime() const
{
    return m_logTime;
}

void
LogViewEntry::setLocalTime(const LogTime& logTime)
{
    m_logTime = logTime;
}

const std::string&
LogViewEntry::getBootId() const
{
    return m_bootId;
}

void
LogViewEntry::setBootId(const std::string& bootId)
{
    m_bootId = bootId;
}

const std::string&
LogViewEntry::getLocation() const
{
    return m_location;
}

void
LogViewEntry::setLocation(const std::string& location)
{
    m_location = location;
}

Level
LogViewEntry::getLevel() const
{
    return m_level;
}

void
LogViewEntry::setLevel(Level level)
{
    m_level = level;
}

} /* namespace log */
} /* namespace psc */


