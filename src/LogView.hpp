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

#include <memory>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <ranges>
#include <exception>
#include <sys/time.h>
#if __cplusplus >= 202002L
#include <chrono>
#define USE_STD_CHRONO
#else
#include <glibmm.h>
#endif

#include "Log.hpp"

namespace psc {
namespace log {

// as i implemented timing with std::chrono first
//   and realized later that c++20 is not available for e.g. debian
//   the defines might be ugly, but at least they allow using the types as value.

#ifdef USE_STD_CHRONO
// Micros are used by systemd, so adapt this here
using LogTimePrecision = std::chrono::microseconds;
using LocalTime = std::chrono::time_point<std::chrono::local_t,LogTimePrecision>;
using LocalDays = std::chrono::local_days;
#else
using LocalTime = Glib::DateTime;   // for earlier gcc versions fall back to glibmm DateTime
using LocalDays = Glib::Date;
#endif

class LogTime;

class LogDays
{
public:
    LogDays() = default;
    LogDays(const LogTime& logTime);
    LogDays(const LogDays& logDays) = default;
    virtual ~LogDays() = default;

    Glib::ustring format(const char* fmt) const;
    void parseIsoDate(const std::string& date);
    int compare(const LogDays& other) const;
    bool isValid() const;
   // for c++20 this can be done with <=>
    bool operator<(const LogDays& other) const;
    bool operator>(const LogDays& other) const;
    bool operator==(const LogDays& other) const;
    static constexpr auto SECONDS_PER_DAY = 24l*60l*60l;

private:
    LocalDays m_localDays;
};


class LogTime
{
public:
    LogTime() = default;
    LogTime(const LocalTime& localTime);
    LogTime(const LogTime& logtime) = default;
    virtual ~LogTime() = default;

    Glib::ustring format(const char* fmt) const;
    LogDays toDays() const;
    bool isValid() const;
    void parseIsoDateTime(const std::string& dateTime);
    LocalTime getLocalTime() const;
    int compare(const LogTime& other) const;
    void parse(const char* fmt, const std::string& val, bool isLocal);
    static LogTime now();
    static LogTime max();
    static LogTime min();
    static LogTime create_usec(uint64_t usec);
    static constexpr auto USEC = 1000000l;
private:
    LocalTime m_localTime;
};

std::ostream&
operator<< (std::ostream& os, const LogTime& logTime);

class LogViewException : public std::exception
{
public:
    LogViewException(const std::string& msg);

    virtual const char * what() const noexcept;
private:
    std::string swhat;
};



class LogView;

typedef std::shared_ptr<LogView> pLogView;

class LogViewIterator;

class LogViewIdentifier;
typedef std::shared_ptr<LogViewIdentifier> pLogViewIdentifier;
class LogViewEntry;
typedef std::shared_ptr<LogViewEntry> pLogViewEntry;

class LogView
{
public:
    LogView() = default;
    LogView(const LogView& orig) = default;
    virtual ~LogView() = default;
    // get the LogView matching the configured logging
    static pLogView create();
    virtual std::vector<pLogViewIdentifier> getIdentifiers() = 0;
    virtual void setQuery(const std::list<pLogViewIdentifier>& query) = 0;
    // there might be a large number of entries, make this a streaming interface
    //   condition: setQuery must have been called before
    virtual LogViewIterator begin() = 0;
    virtual LogViewIterator end() = 0;

    virtual std::string getBootId() = 0;
private:
};



class LogViewEntry
{
public:
    LogViewEntry();
    LogViewEntry(
              const std::string& msg
            , const LogTime& logTime
            , const std::string& bootId
            , const std::string& location
            , Level level);
    LogViewEntry(const LogViewEntry& orig) = default;
    virtual ~LogViewEntry() = default;

    const std::string& getMessage() const;
    void setMessage(const std::string& message);
    const LogTime& getLocalTime() const;
    void setLocalTime(const LogTime& logTime);
    const std::string& getBootId() const;
    void setBootId(const std::string& bootId);
    const std::string& getLocation() const;
    void setLocation(const std::string& location);
    Level getLevel() const;
    void setLevel(Level level);
private:
    std::string m_message;
    LogTime m_logTime;
    std::string m_bootId;
    std::string m_location;
    Level m_level;
};


struct LogViewIterInner
{
    using iterator_category = std::forward_iterator_tag;
    // operator members are not possible with virtual
    virtual void inc() = 0;
    virtual bool equal(const std::shared_ptr<LogViewIterInner>& a) = 0;
    virtual LogViewEntry get() const = 0;
};

typedef std::shared_ptr<LogViewIterInner> pLogViewIterInner;

// as inheritance won't allow flexible typing use a inner instance
class LogViewIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = int64_t;
    using value_type        = LogViewEntry;
    using pointer_type      = pLogViewEntry;
    using reference_type    = LogViewEntry&;

public:
    LogViewIterator(pLogViewIterInner iterInner)
    : m_iterInner{iterInner}
    {
    }
    LogViewIterator(const LogViewIterator& other) = default;
    virtual ~LogViewIterator() = default;
    LogViewIterator operator++()
    {
        m_iterInner->inc();
        m_logEntry.reset();
        return *this;
    }
    LogViewIterator operator++(int)
    {
        LogViewIterator temp = *this;
        m_iterInner->inc();
        m_logEntry.reset();
        return temp;
    }
    value_type operator*()
    {
        return m_iterInner->get();
    }
    pointer_type operator->()
    {
        if (!m_logEntry) {
            m_logEntry = std::make_shared<LogViewEntry>(std::move(m_iterInner->get()));
        }
        return m_logEntry;
    }
    friend bool operator== (const LogViewIterator& a, const LogViewIterator& b)
    {
        return a.m_iterInner->equal(b.m_iterInner);
    }
    friend bool operator!= (const LogViewIterator& a, const LogViewIterator& b)
    {
        return !a.m_iterInner->equal(b.m_iterInner);
    }
private:
    pLogViewIterInner m_iterInner;
    pointer_type m_logEntry;
};

enum class LogViewType
{
     Name
    ,Time
};

class LogViewIdentifier
{
public:
    LogViewIdentifier(LogViewType type, const std::string& name, const std::string& value)
    : m_type{type}
    , m_name{name}
    , m_value{value}
    {
    }
    explicit LogViewIdentifier(const LogViewIdentifier& orig) = delete;
    virtual ~LogViewIdentifier() = default;

    std::string getName() const
    {
        return m_name;
    }
    void setName(const std::string& name)
    {
        m_name = name;
    }
    LogViewType getType()
    {
        return m_type;
    }
    std::string getValue()
    {
        return m_value;
    }
    virtual bool isBootId(const std::string& bootId) = 0;
protected:
    LogViewType m_type;
    std::string m_name;
    std::string m_value;
};



} /* namespace log */
} /* namespace psc */


