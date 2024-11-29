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

#include <chrono>
#include <fstream>
#include <filesystem>

#include "Log.hpp"
#include "LogView.hpp"


namespace psc {
namespace log {

typedef std::chrono::time_point<std::chrono::system_clock> SystemTime;

class LogViewEntryFile;

typedef std::shared_ptr<LogViewEntryFile> pLogViewEntryFile;

class LogViewFile;

typedef std::shared_ptr<LogViewFile> pLogViewFile;

class LogViewFile
: public LogView
, public std::enable_shared_from_this<LogViewFile>
{
public:
    LogViewFile();
    explicit LogViewFile(const LogViewFile& orig) = delete;
    virtual ~LogViewFile() = default;

    std::vector<pLogViewIdentifier> getIdentifiers() override;
    void setQuery(const std::list<pLogViewIdentifier>& query) override;
    // there might be a large number of entries, make this a streaming interface
    //   condition: setQuery must have been called before
    virtual LogViewIterator begin() override;
    virtual LogViewIterator end() override;
    virtual std::string getBasePath();
    std::string getBootId() override;
    virtual pLogViewEntryFile parse(const std::string& line);
    // prefer this creation method
    static pLogViewFile create();

    //static constexpr auto FULL_SCAN_LIMIT = 1024l*1024l;
    static constexpr auto LOG_EXTENSTION = ".log";
protected:
    std::map<LogDays, uint64_t> groupDays(const std::filesystem::path& entry);
    std::list<pLogViewIdentifier> m_query;
private:
};




class LogViewIdFile
: public LogViewIdentifier
{
public:
    // add file size as well and on reading, check it is >=
    //   if it is not the log file has been rolled over.
    LogViewIdFile(LogViewType type, const std::string& path, const std::map<LogDays, uint64_t>& dayMap, uint64_t fileSize, const std::string& name);
    LogViewIdFile(LogViewType type, const std::string& name);
    explicit LogViewIdFile(const LogViewIdFile& logViewIdFile) = delete;
    ~LogViewIdFile() = default;

    std::string getPath() const;
    uint64_t getFileSize() const;
    std::map<LogDays, uint64_t>& getDayMap();
    bool isBootId(const std::string& bootId) override;
private:
    std::string m_path;
    std::map<LogDays, uint64_t> m_dayMap;
    uint64_t m_fileSize;
};

typedef std::shared_ptr<LogViewIdFile> pLogViewIdFile;

struct LogViewFileIterator
: public LogViewIterInner
{
    using iterator_category = std::forward_iterator_tag;
    using value_type        = pLogViewEntry;
    // constructor
    LogViewFileIterator()
    : m_pos{std::numeric_limits<uint64_t>::max()}    // use the maximum allowed value as end mark
    {
    }
    LogViewFileIterator(const std::list<pLogViewIdentifier>& query, const pLogViewFile& logViewFile);
    virtual ~LogViewFileIterator();
    // member functions
    value_type get() const override;

    void inc() override;
    bool equal(const std::shared_ptr<LogViewIterInner>& b) override;
    virtual pLogViewEntry parse();
    static constexpr auto SIZE_LIMIT = 32l*1024l;
protected:
    uint64_t m_pos;
    std::ifstream m_stat;
    pLogViewEntry m_logViewEntry;
    pLogViewFile m_logViewFile;
    LogDays m_viewDay;
};


class LogViewEntryFile
: public LogViewEntry
{
public:
    LogViewEntryFile(const std::string& location, const LogTime& timestamp, const std::string& message, Level level);
    explicit LogViewEntryFile(const LogViewEntryFile& other) = delete;
    ~LogViewEntryFile() = default;

    const std::string& getMessage() override;
    const LogTime& getLocalTime() override;
    const std::string& getBootId() override;
    const std::string& getLocation() override;
    void setLocation(const std::string& location);
    Level getLevel() override;

private:
    std::string m_message;
    std::string m_bootId;
    LogTime m_timestamp;
    std::string m_location;
    Level m_level;
};


} /* namespace log */
} /* namespace psc */
