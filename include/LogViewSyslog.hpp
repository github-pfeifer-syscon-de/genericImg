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

// as this has no Api dependencies allow free usage
//#include "genericimg_config.h"
//#ifdef SYSLOG


#include "LogViewFile.hpp"


namespace psc::log {

class LogViewSyslog;

typedef std::shared_ptr<LogViewSyslog> pLogViewSyslog;

class LogViewSyslog
: public LogViewFile
{
public:
    LogViewSyslog();
    explicit LogViewSyslog(const LogViewSyslog& orig) = delete;
    virtual LogViewIterator begin() override;
    virtual LogViewIterator end() override;

    std::string getBasePath() override;
    // if the format doesn't fit (as there is no universal syslog format) you may need to adapt this method
    LogViewEntry parse(const std::string& line) override;
    // prefer this creation method
    static pLogViewSyslog create();
protected:
private:
};


struct LogViewSyslogIterator
: public LogViewFileIterator
{
    // constructor
    LogViewSyslogIterator()
    : LogViewFileIterator::LogViewFileIterator()
    {
    }
    LogViewSyslogIterator(const std::list<pLogViewIdentifier>& query, const pLogViewFile& logViewFile)
    : LogViewFileIterator::LogViewFileIterator(query, logViewFile)
    {
    }

    LogViewEntry parse() override;
protected:
};


} /* namespace psc::log */

