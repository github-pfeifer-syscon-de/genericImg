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
#include <format>
#include <sys/types.h>
#include <pwd.h>

#include "StringUtils.hpp"
#include "LogViewFile.hpp"


namespace psc {
namespace log {


LogViewFile::LogViewFile()
: LogView::LogView()
{
}

std::vector<pLogViewIdentifier>
LogViewFile::getIdentifiers()
{
    std::vector<pLogViewIdentifier> ret;
    ret.reserve(64);
    std::set<LogDays> days;
    for (const auto& entry : std::filesystem::directory_iterator(getBasePath())) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension();
            if (LOG_EXTENSTION == ext) {     // avoid reading binary files
                std::string dir = entry.path().parent_path().string();
                std::string file = entry.path().filename().string();
                if (groupDays(entry.path(), days)) {    // only list if compatible
                    auto id = std::make_shared<LogViewIdFile>(LogViewType::Name, dir, file);
                    ret.emplace_back(std::move(id));
                }
            }
        }
        // check recursive ?
    }
    //std::cout << "Colleced days" << std::endl;
    for (auto& day : days) {
        //std::cout << "day " << day << std::endl;
        auto isoDate = day.format("%F");
        auto id = std::make_shared<LogViewIdFile>(LogViewType::Time, "", isoDate);
        ret.emplace_back(std::move(id));
    }
    return ret;
}

bool
LogViewFile::groupDays(const std::filesystem::path& path, std::set<LogDays>& days)
{
    bool inserted = false;
    std::ifstream stat;
    std::ios_base::iostate exceptionMask = stat.exceptions() | std::ios::failbit | std::ios::badbit ;   // | std::ios::eofbit
    stat.exceptions(exceptionMask);
    try {
        auto size = std::filesystem::file_size(path);
        stat.open(path.generic_string()); // open in text-mode
        //std::cout << "reading " << path << std::endl;
        long skip = 0;
        if (size > FULL_SCAN_LIMIT) {
            skip = size / 128l;    // check 128 places
        }
        while (!stat.eof()) {
            std::string line;
            std::getline(stat, line);
            if (line.starts_with(' ')) {    // specific for LogViewFile (skip location)
                if (stat.eof()) {
                    break;
                }
                std::getline(stat, line);
            }
            //std::cout << "parsing " << line << std::endl;
            auto pLogViewFile = parse(line);
            if (pLogViewFile
             && pLogViewFile->getLocalTime().isValid()) {
                auto dayDate = pLogViewFile->getLocalTime().toDays();
                //std::cout << "dayDate " << dayDate <<std::endl;
                auto exist = days.find(dayDate);
                if (exist == days.end()) {
                    days.insert(dayDate);
                    inserted = true;
                }
            }
            if (skip > 0) {
                stat.seekg(skip, std::ios_base::cur);
                if (stat.tellg() >= static_cast<std::streamoff>(size)) {
                    break;
                }
                while (!stat.eof()) {    // search proper line start
                    int c = stat.get();
                    if (c == EOF || c == '\n') {
                        break;
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "LogViewFile::groupDays"
                  << " path " << path
                  << " end " << e.what() << std::endl;
    }
    if (stat.is_open()) {
        stat.close();
    }
    return inserted;
}

std::string
LogViewFile::getBasePath()
{
    const char *homedir = getpwuid(getuid())->pw_dir;
    std::filesystem::path path{homedir};
    path /= "log";
    return path.string();
}


void
LogViewFile::setQuery(const std::list<pLogViewIdentifier>& query)
{
    if (query.empty()) {
        throw LogViewException("No query elements, not usable");
    }
    m_query = query;
}


LogViewIterator
LogViewFile::begin()
{
    auto logViewFile = shared_from_this();
    auto logViewFileIterator = std::make_shared<LogViewFileIterator>(m_query, logViewFile);
    logViewFileIterator->inc(); // ready first entry
    return LogViewIterator(logViewFileIterator);
}

LogViewIterator
LogViewFile::end()
{
    auto logViewFileIterator = std::make_shared<LogViewFileIterator>();
    return LogViewIterator(logViewFileIterator);
}

pLogViewFile
LogViewFile::create()
{
    return std::make_shared<LogViewFile>();
}

// for files the closes we get to a bootId is the current date
std::string
LogViewFile::getBootId()
{
    auto now = std::chrono::system_clock::now();    // as a bootId ~equivalent use current day
    auto isoNow = std::format("{:%F}", now);
    return isoNow;
}

pLogViewEntryFile
LogViewFile::parse(const std::string& line)
{
    pLogViewEntryFile logViewEntry;
    if (line.length() > 23 && !line.starts_with(' ')) { // if we get a blank this is out of sync
        std::string dateTime = line.substr(0, 23);
        LogTime timestamp;
        timestamp.parseIsoDateTime(dateTime);
        Level level = Level::Info;
        size_t levelStart = 24;
        size_t levelEnd = line.find(' ', levelStart);
        if (levelEnd != line.npos) {
            auto levelName = line.substr(levelStart, levelEnd-levelStart);
            level = Log::getLevel(levelName);
            ++levelEnd;
            std::string message = line.substr(levelEnd);
            StringUtils::ltrim(message);
            logViewEntry = std::make_shared<LogViewEntryFile>("", timestamp, message, level);
        }
    }
    return logViewEntry;
}


LogViewIdFile::LogViewIdFile(LogViewType type, const std::string& path, const std::string& name)
: LogViewIdentifier(type, name, name)
, m_path{path}
{
}

std::string
LogViewIdFile::getPath() const
{
    return m_path;
}

bool
LogViewIdFile::isBootId(const std::string& bootId)
{
    return m_type == LogViewType::Time
        && bootId == getName();
}

LogViewFileIterator::LogViewFileIterator(const std::list<pLogViewIdentifier>& query, const pLogViewFile& logViewFile)
: m_logViewFile{logViewFile}
{
    std::filesystem::path path;
    for (auto& id : query) {
        auto fileId = std::dynamic_pointer_cast<LogViewIdFile>(id);
        if (!fileId ) {
            throw LogViewException("Only LogViewIdFile elements are supported for query");
        }
        if (LogViewType::Name == fileId->getType()) {
            path = std::filesystem::path{fileId->getPath()};
            path /= fileId->getName();
        }
        else if (LogViewType::Time == fileId->getType()) {
            m_viewDay.parseIsoDate(fileId->getName());
        }
    }
    if (path.empty()) {
        throw LogViewException("At least a name element is required for query");
    }
    std::ios_base::iostate exceptionMask = m_stat.exceptions() | std::ios::failbit | std::ios::badbit | std::ios::eofbit;
    m_stat.exceptions(exceptionMask);
    try {
        //auto size = std::filesystem::file_size(path);
        m_stat.open(path.string()); // open in text-mode
        m_pos = 0l;
        // prevent spooling megabytes of logs
        //if (size > 2l*SIZE_LIMIT) {
        ////std::cout << "Skipping to " << size-SIZE_LIMIT << std::endl;
        //   m_stat.seekg(size-SIZE_LIMIT, std::ios_base::beg);
        //   int c;
        //   while ((c = m_stat.get()) != '\n') {    // search proper line start
        //      // keep reading eof will throw exception
        //   }
        //}
    }
    catch (const std::exception& e) {  // something went wrong "jump" to end
        //std::cout << "LogViewFileIterator::LogViewFileIterator"
        //          << " query " << match
        //          << " end " << e.what() << std::endl;
        m_pos = std::numeric_limits<uint64_t>::max();
    }
}

LogViewFileIterator::~LogViewFileIterator()
{
    if (m_stat.is_open()) {
        m_stat.close();
    }
}

LogViewFileIterator::value_type
LogViewFileIterator::get() const
{
    return m_logViewEntry;
}

void
LogViewFileIterator::inc()
{
    if (m_pos < std::numeric_limits<uint64_t>::max()
     && !m_stat.eof()) {
        try {
            LogDays logDay;
            do {
                m_logViewEntry = parse();
                if (m_viewDay.isValid()) { 
                    logDay = m_logViewEntry->getLocalTime().toDays();
                    //std::cout << "LogViewFileIterator::inc"
                    //          << " logDay " << logDay
                    //          << " m_viewDay " << m_viewDay << std::endl;
                    if (logDay.compare(m_viewDay) > 0) {
                        m_pos = std::numeric_limits<uint64_t>::max();
                        return;
                    }
                }
                else {
                    break;
                }
            }
            while (logDay.compare(m_viewDay) < 0);
            if (m_pos < std::numeric_limits<uint64_t>::max()) {
                ++m_pos;
            }
        }
        catch (const std::ios_base::failure& e) {  // something went wrong "jump" to end
            m_pos = std::numeric_limits<uint64_t>::max();
        }
    }
    else {
        m_pos = std::numeric_limits<uint64_t>::max();
    }
}

bool
LogViewFileIterator::equal(const std::shared_ptr<LogViewIterInner>& b)
{
    auto sysdB = std::dynamic_pointer_cast<LogViewFileIterator>(b);
    if (sysdB) {
        return m_pos == sysdB->m_pos;
    }
    return false;   // or return true to make end check successful
}


pLogViewEntry
LogViewFileIterator::parse()
{
    //2024-10-19 17:22:05.996 Deb          Weather.cpp: 45 pixbuf stream 0x5dd68664f480
    //                               Glib::RefPtr<Gdk::Pixbuf> WeatherImageRequest::get_pixbuf()
    std::string line;
    std::getline(m_stat, line);
    if (line.starts_with(' ')) {    // guess we hit a location line
        std::getline(m_stat, line); // so try again
    }
    auto logViewEntry = m_logViewFile->parse(line);
    if (logViewEntry && m_stat.peek() == ' ') {
        std::getline(m_stat, line);
        StringUtils::ltrim(line);
        logViewEntry->setLocation(line);
    }
    return logViewEntry;
}



LogViewEntryFile::LogViewEntryFile(const std::string& location, const LogTime& timestamp, const std::string& message, Level level)
: LogViewEntry()
, m_message{message}
, m_timestamp{timestamp}
, m_location{location}
, m_level{level}
{
}

const std::string&
LogViewEntryFile::getMessage()
{
    return m_message;
}

const LogTime&
LogViewEntryFile::getLocalTime()
{
    return m_timestamp;
}

const std::string&
LogViewEntryFile::getBootId()
{
    return m_bootId;
}

const std::string&
LogViewEntryFile::getLocation()
{
    return m_location;
}

void
LogViewEntryFile::setLocation(const std::string& location)
{
    m_location = location;
}

Level LogViewEntryFile::getLevel()
{
    return m_level;
}

} /* namespace log */
} /* namespace psc */
