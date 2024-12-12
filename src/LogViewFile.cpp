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
#include <sys/types.h>
#include <glibmm.h>     // using Glib::get_home_dir

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
                std::map<LogDays, uint64_t> mapDays = groupDays(entry.path());
                if (!mapDays.empty()) {    // only list if compatible
                    auto fileSize = std::filesystem::file_size(entry.path());
                    auto id = std::make_shared<LogViewIdFile>(LogViewType::Name, dir, mapDays, fileSize, file);
                    ret.emplace_back(std::move(id));
                    for (auto& entry : mapDays) {
                        auto mapDay = entry.first;
                        auto day = days.find(mapDay);
                        if (day == days.end()) {
                            days.insert(mapDay);
                        }
                    }
                }
            }
        }
        // check recursive ?
    }
    //std::cout << "Colleced days" << std::endl;
    for (auto& day : days) {
        //std::cout << "day " << day << std::endl;
        auto isoDate = day.format("%F");
        auto id = std::make_shared<LogViewIdFile>(LogViewType::Time, isoDate);
        ret.emplace_back(std::move(id));
    }
    return ret;
}

// as we already doing the effort of scanning the file,
//   do not optimize (no skipping), but record the position of each day
std::map<LogDays, uint64_t>
LogViewFile::groupDays(const std::filesystem::path& path)
{
    std::map<LogDays, uint64_t> map;
    std::ifstream stat;
    std::ios_base::iostate exceptionMask = stat.exceptions() | std::ios::failbit | std::ios::badbit | std::ios::eofbit;
    stat.exceptions(exceptionMask);
    try {
        stat.open(path.generic_string()); // open in text-mode
        //std::cout << "reading " << path << std::endl;
        while (!stat.eof()) {
            auto pos = stat.tellg();        // capture position for start of line
            std::string line;
            std::getline(stat, line);
            if (line.length() >= 1 && line[0] == ' ') {    // specific for LogViewFile (skip location)
                if (stat.eof()) {
                    break;
                }
                std::getline(stat, line);
            }
            //std::cout << "parsing " << line << std::endl;
            auto logViewFile = parse(line);
            if (logViewFile.getLocalTime().isValid()) {
                auto dayDate = logViewFile.getLocalTime().toDays();
                //std::cout << "dayDate " << dayDate <<std::endl;
                auto exist = map.find(dayDate);
                if (exist == map.end()) {
                    map.insert(std::pair(dayDate, pos));
                }
            }
        }
    }
    catch (const std::exception& e) {
        if (!stat.eof()) {
            std::cout << "LogViewFile::groupDays"
                      << " path " << path
                      << " end " << e.what() << std::endl;
        }
    }
    if (stat.is_open()) {
        stat.close();
    }
    return map;
}

std::string
LogViewFile::getBasePath()
{
    std::filesystem::path path{Glib::get_home_dir()};
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
    LogTime now = LogTime::now();
    auto isoNow = now.format("%F");
    return isoNow;
}

LogViewEntry
LogViewFile::parse(const std::string& line)
{
    LogViewEntry logViewEntry;
    if (line.length() > 23 && line[0] != ' ') { // if we get a blank this is out of sync
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
            logViewEntry.setLocalTime(timestamp);
            logViewEntry.setMessage(message);
            logViewEntry.setLevel(level);
        }
    }
    return logViewEntry;
}

LogViewIdFile::LogViewIdFile(LogViewType type, const std::string& path, const std::map<LogDays, uint64_t>& dayMap, uint64_t fileSize, const std::string& name)
: LogViewIdentifier(type, name, name)
, m_path{path}
, m_dayMap{dayMap}
, m_fileSize{fileSize}
{
}


LogViewIdFile::LogViewIdFile(LogViewType type, const std::string& name)
: LogViewIdentifier(type, name, name)
{
}

std::string
LogViewIdFile::getPath() const
{
    return m_path;
}

uint64_t
LogViewIdFile::getFileSize() const
{
    return m_fileSize;
}

std::map<LogDays, uint64_t>&
LogViewIdFile::getDayMap()
{
    return m_dayMap;
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
    pLogViewIdFile nameId;
    for (auto& id : query) {
        auto fileId = std::dynamic_pointer_cast<LogViewIdFile>(id);
        if (!fileId ) {
            throw LogViewException("Only LogViewIdFile elements are supported for query");
        }
        if (LogViewType::Name == fileId->getType()) {
            nameId = fileId;
        }
        else if (LogViewType::Time == fileId->getType()) {
            m_viewDay.parseIsoDate(fileId->getName());
        }
    }
    if (!nameId) {
        throw LogViewException("At least a name element is required for query");
    }
    std::filesystem::path path{nameId->getPath()};
    path /= nameId->getName();
    uint64_t pos = 0ul;
    if (m_viewDay.isValid()) {
        std::map<LogDays, uint64_t> mapDays = nameId->getDayMap();
        auto posEntry = mapDays.find(m_viewDay);
        if (posEntry != mapDays.end()) {
            pos = (*posEntry).second;
        }
    }
    std::ios_base::iostate exceptionMask = m_stat.exceptions() | std::ios::failbit | std::ios::badbit | std::ios::eofbit;
    m_stat.exceptions(exceptionMask);
    try {
        auto fileSize = std::filesystem::file_size(path);
        m_stat.open(path.string()); // open in text-mode
        m_pos = 0l;
        if (pos > 0ul
         && fileSize >= nameId->getFileSize()) {    // if file has been rolled over, don't skip
            m_stat.seekg(pos, std::ios_base::beg);
            m_pos = pos;
        }
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
                    logDay = m_logViewEntry.getLocalTime().toDays();
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


LogViewEntry
LogViewFileIterator::parse()
{
    //2024-10-19 17:22:05.996 Deb          Weather.cpp: 45 pixbuf stream 0x5dd68664f480
    //                               Glib::RefPtr<Gdk::Pixbuf> WeatherImageRequest::get_pixbuf()
    std::string line;
    std::getline(m_stat, line);
    if (line.length() >= 1 && line[0] == ' ') {    // guess we hit a location line
        std::getline(m_stat, line); // so try again
    }
    auto logViewEntry = m_logViewFile->parse(line);
    if (m_stat.peek() == ' ') {
        std::getline(m_stat, line);
        StringUtils::ltrim(line);
        logViewEntry.setLocation(line);
    }
    return logViewEntry;
}


} /* namespace log */
} /* namespace psc */
