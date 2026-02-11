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

#include "genericimg_config.h"
#ifdef SYSDLOG

#include <chrono>
#include <ctime>
#include <iterator> // For std::forward_iterator_tag
#include <limits>

#define SD_JOURNAL_SUPPRESS_LOCATION    // avoid default location fields
#include <systemd/sd-journal.h>

#include "LogView.hpp"



namespace psc::log {

class LogViewSysdJournal
{
public:
    LogViewSysdJournal(int flags);
    explicit LogViewSysdJournal(const LogViewSysdJournal& orig) = delete;
    virtual ~LogViewSysdJournal();
    sd_journal* getJournal();
    sd_journal* removeJournal();

    const LogTime getLocalTime();
    // get the content of field as provided usually: key=value
    std::string getField(const char* field);
    // get the value of field
    std::string getFieldValue(const char* field);
    int getError();
    bool seekHead();

private:
    sd_journal* m_j;
    int m_ret;
};

typedef std::shared_ptr<LogViewSysdJournal> pLogViewSysdJournal;

class LogViewSysd
: public LogView
{
public:
    LogViewSysd();
    explicit LogViewSysd(const LogViewSysd& orig) = delete;
    virtual ~LogViewSysd() = default;

    int getFlags();
    std::vector<pLogViewIdentifier> getIdentifiers() override;
    void setQuery(const std::list<pLogViewIdentifier>& query) override;
    LogViewIterator begin() override;
    LogViewIterator end() override;

    std::string getBootId() override;
    static constexpr auto BOOT_INFO_CONF = "boot_info.conf";
    static constexpr auto BOOT_INFO_GRP = "boot_info";
    static constexpr auto MIN_SUFFIX = "_Min";
    static constexpr auto MAX_SUFFIX = "_Max";
protected:
    void queryUnique(const char* type, std::vector<pLogViewIdentifier>& vecIds);
    void boot_info(std::vector<pLogViewIdentifier>& mapBootId);
private:
    std::list<pLogViewIdentifier> m_query;
};

typedef std::shared_ptr<LogViewSysd> pLogViewSysd;


struct LogViewSysdJournalIterator
: public LogViewIterInner
{
    using iterator_category = std::forward_iterator_tag;
    using value_type        = LogViewEntry;
    using reference_type    = LogViewEntry&;
    // constructor
    LogViewSysdJournalIterator()
    : m_pos{std::numeric_limits<uint64_t>::max()}    // use the maximum allowed value as end mark
    {
    }
    LogViewSysdJournalIterator(const std::list<pLogViewIdentifier>& query, LogViewSysd& logViewIdSysd);

    // member functions
    value_type get() const override;

    void inc() override
    {
        if (m_pos < std::numeric_limits<uint64_t>::max()
         && sd_journal_next(m_logViewSysdJournal->getJournal()) > 0) {
            ++m_pos;
        }
        else {
            m_pos = std::numeric_limits<uint64_t>::max();
        }
    }
    bool equal(const std::shared_ptr<LogViewIterInner>& b) override
    {
        auto sysdB = std::dynamic_pointer_cast<LogViewSysdJournalIterator>(b);
        if (sysdB) {
            return m_pos == sysdB->m_pos;
        }
        return false;
    };
private:
    uint64_t m_pos;
    pLogViewSysdJournal m_logViewSysdJournal;
};


class LogViewIdSysd
: public LogViewIdentifier
{
public:
    LogViewIdSysd(LogViewType type, const std::string& name, const std::string& query);
    explicit LogViewIdSysd(const LogViewIdSysd& orig) = delete;
    virtual ~LogViewIdSysd() = default;

    std::string getQuery() const;
    bool isBootId(const std::string& bootId);
protected:

private:
    std::string m_query;
    std::string m_bootId;
};



} /* namespace psc::log */

#endif
