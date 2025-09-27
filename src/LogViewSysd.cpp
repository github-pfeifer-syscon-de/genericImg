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
#include "config.h"
#ifdef SYSDLOG

#include <iostream>
#include <ctime>
#include <string>
#include <glibmm.h>
#include <giomm/file.h>

#include "psc_format.hpp"
#include "StringUtils.hpp"
#include "LogViewSysd.hpp"

namespace psc {
namespace log {


// encapsulate the access of the journal resource
LogViewSysdJournal::LogViewSysdJournal(int flags)
{
    m_ret = sd_journal_open(&m_j, flags);
    if (m_ret != 0) {
        m_j = nullptr;
        throw LogViewException(psc::fmt::format("Error on open_journal {}", strerror(m_ret)));
    }
}

LogViewSysdJournal::~LogViewSysdJournal()
{
    if (m_j) {
        sd_journal_close(m_j);
    }
}

sd_journal*
LogViewSysdJournal::getJournal()
{
    return m_j;
}

// get the journal (the responsibity to close is transfered somewhere else)
sd_journal*
LogViewSysdJournal::removeJournal()
{
    sd_journal* j = m_j;
    m_j = nullptr;
    return j;
}

int
LogViewSysdJournal::getError()
{
    return m_ret;
}

bool
LogViewSysdJournal::seekHead()
{
    // use both functions so the cursor is positioned
    return sd_journal_seek_head(m_j) == 0
        && sd_journal_next(m_j) > 0;
}


const LogTime
LogViewSysdJournal::getLocalTime()
{
    uint64_t usec{0l};
    m_ret = sd_journal_get_realtime_usec(m_j, &usec);
    return LogTime::create_usec(m_ret == 0 ? usec : 0ul);
}

std::string
LogViewSysdJournal::getField(const char* field)
{
    const char *d;
    size_t l;
    m_ret = sd_journal_get_data(m_j, field, (const void **)&d, &l);
    if (m_ret == 0) {
        auto msg = std::string{d, l};
        return msg;
    }
    return "";
}

std::string
LogViewSysdJournal::getFieldValue(const char* field)
{
    auto msg = getField(field);
    if (!msg.empty()) {
        auto pos = msg.find('=');
        if (pos != msg.npos) {
            msg = msg.substr(pos+1);
        }
        return msg;
    }
    //std::cerr << "Failed to read " << field << " " << strerror(-ret) << std::endl;
    return "";
}

LogViewSysd::LogViewSysd()
: LogView()
{
}

int
LogViewSysd::getFlags()
{
    return SD_JOURNAL_LOCAL_ONLY|SD_JOURNAL_CURRENT_USER;
}

void
LogViewSysd::queryUnique(const char* type, std::vector<pLogViewIdentifier>& vecIds)
{
    LogViewSysdJournal journal(getFlags());
    int ret = sd_journal_query_unique(journal.getJournal(), type);
    if (ret < 0) {
        throw LogViewException(psc::fmt::format("Error on sd_journal_query_unique {}", strerror(journal.getError())));
    }
    const void *d;
    size_t l;
    SD_JOURNAL_FOREACH_UNIQUE(journal.getJournal(), d, l) {
        auto typeKey = std::string{static_cast<const char*>(d), l};
        //std::cout << "Query unique " << type << " got " << typeKey << std::endl;
        auto pos = typeKey.find('=');
        auto id = typeKey;
        if (pos != typeKey.npos) {
            id = typeKey.substr(pos+1);
        }
        if (!id.starts_with("systemd-coredump@")) {     // exclude these
            LogViewType logViewType = typeKey.starts_with("_BOOT_ID=")
                                        ? LogViewType::Time
                                        : LogViewType::Name;
            auto sysId = std::make_shared<LogViewIdSysd>(logViewType, id, typeKey);
            vecIds.emplace_back(std::move(sysId));
        }
    }
}

// this is for some cases time consuming (search all log entries for a bootId to get the boot time )
//    for subsequent calls the value read from a config file.
//    I used a classic hard-disc and it is noticeable (for faster medias this might be no issues).
//    But if this might be issue for you do this in thread (and reduce the id's to current if too many are missing).
void
LogViewSysd::boot_info(std::vector<pLogViewIdentifier>& vecBootId)
{
    auto logView = psc::log::LogView::create();
    auto bootId = logView->getBootId();
    //std::cout << "Current boot " << bootId << std::endl;
    Glib::KeyFile keyFile;
    std::string confPath = g_canonicalize_filename(BOOT_INFO_CONF, Glib::get_user_config_dir().c_str());
    auto confFile = Gio::File::create_for_path(confPath);
    if (confFile->query_exists()) {
        try {
            keyFile.load_from_file(confPath, Glib::KeyFileFlags::KEY_FILE_KEEP_COMMENTS);
        }
        catch (const Glib::Error& err) {
            std::cout << "Error loading " << confPath << std::endl;
        }
    }
    bool added = false;
    size_t keyEntries = 0;
    if (keyFile.has_group(BOOT_INFO_GRP)) {
        keyEntries = keyFile.get_keys(BOOT_INFO_GRP).size();
    }
    if (vecBootId.size() > keyEntries * 3 / 4) {
        std::cout << "We are building a list of boot " << vecBootId.size()
                  << " times, available " << keyEntries
                  << " this make take some time." << std::endl;
    }
    for (auto& pLogViewIdentifier : vecBootId) {
        auto confKey = pLogViewIdentifier->getName() + MIN_SUFFIX;
        if (keyFile.has_group(BOOT_INFO_GRP) &&
            keyFile.has_key(BOOT_INFO_GRP,confKey)) {
            auto dateValueMin = keyFile.get_string(BOOT_INFO_GRP, confKey);
            pLogViewIdentifier->setName(dateValueMin);
        }
        else {
            LogViewSysd logView;
            std::list<psc::log::pLogViewIdentifier> query;
            auto id = pLogViewIdentifier->getName();
            auto logViewBoot = std::make_shared<LogViewIdSysd>(LogViewType::Time, id, "_BOOT_ID=" + id);
            query.push_back(logViewBoot);
            logView.setQuery(query);
            auto min = LogTime::max();
            auto max = LogTime::min();
            for (auto iter = logView.begin(); iter != logView.end(); ++iter) {
                auto entry = *iter;
                auto& time = entry.getLocalTime();
                if (min.compare(time) > 0) {
                    min = time;
                }
                if (max.compare(time) < 0) {
                    max = time;
                }
            }
            keyFile.set_string(BOOT_INFO_GRP, id + MIN_SUFFIX, min.format("%F %T"));
            keyFile.set_string(BOOT_INFO_GRP, id + MAX_SUFFIX, max.format("%F %T"));
            pLogViewIdentifier->setName(min.format("%F %T"));
            added = true;
        }
        //std::cout << "logId " << logViewId->getName() << std::endl;
        //const std::time_t tmin = std::chrono::system_clock::to_time_t(minMax.min);
        //const std::time_t tmax = std::chrono::system_clock::to_time_t(minMax.max);
        //std::array<char, 64> smin;
        //auto sminlen = std::strftime(&smin[0], smin.size(), "%F %T", std::gmtime(&tmin));
        //std::array<char, 64> smax;
        //auto smaxlen = std::strftime(&smax[0], smax.size(), "%F %T", std::gmtime(&tmax));
    }
    if (added) {
        try {
            keyFile.save_to_file(confPath);
        }
        catch (const Glib::Error& err) {
            std::cout << "Error saving " << confPath << std::endl;
        }
    }
}


std::vector<pLogViewIdentifier>
LogViewSysd::getIdentifiers()
{
    std::vector<pLogViewIdentifier> vecIds;
    vecIds.reserve(1024);
    queryUnique("_BOOT_ID", vecIds);
    boot_info(vecIds);
    queryUnique("SYSLOG_IDENTIFIER", vecIds);
    queryUnique("_SYSTEMD_UNIT", vecIds);
    std::ranges::sort(vecIds, [](const pLogViewIdentifier& a, const pLogViewIdentifier& b)
    {
        auto atype = a->getType();
        auto btype = b->getType();
        auto cmp = atype<=>btype;
        if (cmp != 0) {
            return cmp > 0;
        }
        auto aname = a->getName();
        auto bname = b->getName();
        cmp = aname<=>bname;
        return cmp > 0;
    });
    return vecIds;
}

void
LogViewSysd::setQuery(const std::list<pLogViewIdentifier>& query)
{
    m_query = query;
}


std::string
LogViewSysd::getBootId()
{
    std::string strBootId;
    sd_id128_t bootId;
    int ret = sd_id128_get_boot(&bootId);
    if (ret != 0) {
        std::cout << "error sd_id128_get_boot " << strerror(ret) << std::endl;
        return "";
    }
    // these function may be an alternative
    //#include <systemd/sd-id128.h>
    //#define SD_ID128_TO_STRING(id) …
    //char *sd_id128_to_string(	sd_id128_t id, char s[static SD_ID128_STRING_MAX]);
    strBootId.reserve(32);
    for (uint32_t i = 0; i < sizeof(bootId.bytes); ++i) {
        strBootId += psc::fmt::format("{:02x}", bootId.bytes[i]);
    }
    return strBootId;
}


LogViewIterator
LogViewSysd::begin()
{
    if (m_query.empty()) {
        throw LogViewException("No query elements, begin not allowed");
    }
    auto innerIter = std::make_shared<LogViewSysdJournalIterator>(m_query, *this);
    return LogViewIterator(innerIter);
}
//
LogViewIterator
LogViewSysd::end()
{
    auto innerIter = std::make_shared<LogViewSysdJournalIterator>();
    return LogViewIterator(innerIter);
}

LogViewIdSysd::LogViewIdSysd(LogViewType type, const std::string& name, const std::string& query)
: LogViewIdentifier(type, name, "")
, m_query{query}
{
    size_t pos = query.find('=');
    if (pos > 0) {
        ++pos;
        m_value = query.substr(pos);
    }
}

std::string
LogViewIdSysd::getQuery() const
{
    return m_query;
}

bool
LogViewIdSysd::isBootId(const std::string& bootId)
{
    return m_type == LogViewType::Time
        && m_query.ends_with(bootId);
}


LogViewSysdJournalIterator::LogViewSysdJournalIterator(const std::list<pLogViewIdentifier>& query, LogViewSysd& logViewIdSysd)
{
    m_logViewSysdJournal = std::make_shared<LogViewSysdJournal>(logViewIdSysd.getFlags());
    for (auto& logViewId : query) {
        auto sysDId = std::dynamic_pointer_cast<LogViewIdSysd>(logViewId);
        if (!sysDId) {
            throw LogViewException("Only LogViewIdSysd elements are supported for query");
        }
        auto match = sysDId->getQuery();
        int ret = sd_journal_add_match(m_logViewSysdJournal->getJournal(), match.c_str(), 0);
        if (ret != 0) {
            throw LogViewException(psc::fmt::format("Error on sd_journal_add_match {}", strerror(ret)));
        }
    }
    if (m_logViewSysdJournal->seekHead()) {
        m_pos = 0l;
    }
    else {  // something went wrong "jump" to end
        m_pos = std::numeric_limits<uint64_t>::max();
    }
}

LogViewSysdJournalIterator::value_type
LogViewSysdJournalIterator::get() const
{
    LogViewEntry logViewEntry;
    // sd_journal_enumerate_available_data()
    // all at once SD_JOURNAL_FOREACH_DATA()
    logViewEntry.setMessage(m_logViewSysdJournal->getFieldValue("MESSAGE"));
    logViewEntry.setLocalTime(m_logViewSysdJournal->getLocalTime());
    logViewEntry.setBootId(m_logViewSysdJournal->getFieldValue("_BOOT_ID"));
    auto prio = m_logViewSysdJournal->getFieldValue("PRIORITY"); // between 0 ("emerg") and 7 ("debug")
    int nprio = std::stoi(prio);
    logViewEntry.setLevel(static_cast<Level>(nprio));
    const auto locationFile = m_logViewSysdJournal->getFieldValue("CODE_FILE");
    const auto locationLine = m_logViewSysdJournal->getFieldValue("CODE_LINE");
    const auto locationFunc = m_logViewSysdJournal->getFieldValue("CODE_FUNC");

    std::string location;
    if (!locationFile.empty()) {
        location.reserve(64);
        location = locationFile;
    }
    if (!locationLine.empty()) {
        location += ":" + locationLine;
    }
    if (!locationFunc.empty()) {
        location += " " + locationFunc;
    }
    logViewEntry.setLocation(location);
    return logViewEntry;
}


} /* namespace log */
} /* namespace psc */


#endif
