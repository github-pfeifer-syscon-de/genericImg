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

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <format>
#include <memory>

#include "genericimg_config.h"
#include "Log.hpp"
#include "LogView.hpp"
#include "LogViewFile.hpp"
#include "LogViewSysd.hpp"
#include "LogViewSyslog.hpp"
#include "DateUtils.hpp"

static bool
test_log()
{
    const std::string logFile{"test.log"};
    std::cout << "Log using " << logFile << std::endl;
    std::remove(logFile.c_str());
    auto log = psc::log::Log::create("test", psc::log::Type::File);
    auto plugin = log->getPlugin();
    auto filePlug = std::dynamic_pointer_cast<psc::log::FilePlugin>(plugin);
    auto fileLog = Gio::File::create_for_path(logFile);
    filePlug->createLogFile(fileLog);   // dont write default dest on testing
    int n = 15;
    psc::log::Log::logAdd(psc::log::Level::Info, [&] {  // try to get best of two worlds, format checking at compile time + lazy evaluation
                              std::cout << "Log called" << std::endl;
                              return std::format("logged {:#08x}!", n);
                          });
    log->close();   // ensure not "buffered"
    auto fs = std::ifstream(logFile);
    if (fs) {
        std::string buf(80, '\0');
        std::getline(fs, buf);
        fs.close();
        std::cout << "Log buf: " << buf << std::endl;
        auto exp_s{"logged 0x00000f!"};
        auto found = buf.find(exp_s);
        auto exp_n{7u};
        if (buf.length() < exp_n || found == buf.npos)  {
            std::cout << "Log expected at least "
                      << exp_n << " got " << buf.length()
                      << " or not found " << exp_s
                      << std::endl;
            return false;
        }
    }
    else {
        std::cout << "Log file not readable? " << std::endl;
        return false;
    }
    std::remove(logFile.c_str());

    return true;
}

static bool
parse_test()
{
    Glib::DateTime dateTime = psc::utl::DateUtils::parse("%b %d %Y %T", "May 11 2024 10:40:48", Glib::TimeZone::create_local());
    Glib::DateTime testLocal = Glib::DateTime::create_local(2024, 5, 11, 10, 40, 48);
    if (dateTime.compare(testLocal) != 0) {
        std::cout << "dateTime " << dateTime.format_iso8601()
                  << " expected " << testLocal.format_iso8601() << std::endl;
        return false;
    }

    dateTime = psc::utl::DateUtils::parse("%FT%T", "2024-10-19T17:22:05.996");
    Glib::DateTime testUtc = Glib::DateTime::create_utc(2024, 10, 19, 17, 22, 5.996);
    if (dateTime.compare(testUtc) != 0) {
        std::cout << "dateTime " << dateTime.format_iso8601()
                  << " expected " << testUtc.format_iso8601() << std::endl;
        return false;
    }
    return true;
}

// this test is for manual control of the output
static bool
test_logview()
{
    auto logView = psc::log::LogView::create();
    auto bootId = logView->getBootId();
    std::cout << "Current boot \"" << bootId << "\"" << std::endl;
    auto ids = logView->getIdentifiers();
    psc::log::pLogViewIdentifier logViewBoot;
    for (auto logViewId : ids) {
        if (logViewId->isBootId(bootId)) {
            logViewBoot = logViewId;
            break;
        }
    }
    if (!logViewBoot) {
        std::cout << "No entry matched " << bootId << std::endl;
        return false;
    }
    for (auto& logViewId : ids) {
        if (logViewId->getType() == psc::log::LogViewType::Name
         && ("monglmm" == logViewId->getName()
         || "pacman.log" == logViewId->getName())) {
            std::list<psc::log::pLogViewIdentifier> query;
            query.push_back(logViewId);
            query.push_back(logViewBoot);
            logView->setQuery(query);
            for (auto iter = logView->begin(); iter != logView->end(); iter.operator++()) {
                auto entry = *iter;
                std::cout << "   time " << entry.getLocalTime() << "\n"
                          << "   msg " << entry.getMessage() << "\n"
                          << "   loc " << entry.getLocation() << std::endl;
            }
        }
    }
    return true;
}

static bool
log_test_syslog()
{
    std::cout << "log_test_syslog" << std::endl;
    std::string logFile{"syslog.log"};
    std::string line{"May 11 10:40:48 scrooge disk-health-nurse[26783]: [ID 702911 user.error] m:SY-mon-full-500 c:H : partition health\n"};
    auto fs = std::ofstream(logFile, std::ios::binary);
    if (fs) {
        fs.write(line.c_str(), line.length());
        fs.close();
        auto fileSize = std::filesystem::file_size(std::filesystem::path{logFile});
        auto logViewSyslog = psc::log::LogViewSyslog::create();
        std::map<psc::log::LogDays, uint64_t> mapDays;
        auto logViewId = std::make_shared<psc::log::LogViewIdFile>(psc::log::LogViewType::Name, ".", mapDays, fileSize, logFile);
        std::list<psc::log::pLogViewIdentifier> query;
        query.emplace_back(std::move(logViewId));
        logViewSyslog->setQuery(query);
        auto cnt = 0u;
        for (auto iter = logViewSyslog->begin(); iter != logViewSyslog->end(); ++iter) {
            auto entry = *iter;
            std::cout << "Level " << psc::log::Log::getLevel(entry.getLevel()) << "\n"
                  << " message " << entry.getMessage() << "\n"
                  << " time " << entry.getLocalTime()  << "\n"
                  << " location " << entry.getLocation() << std::endl;
            ++cnt;
        }
        std::remove(logFile.c_str());
        if (cnt != 1u) {
            std::cout << "log_test_file expected cnt 1 was " << cnt << std::endl;
            return false;
        }
    }
    else {
        return false;
    }
    return true;
}

static bool
log_test_file()
{
    std::cout << "log_test_file" << std::endl;
    std::string logFile{"file.log"};
    std::string line{"2024-10-19 17:22:05.996 Deb          Weather.cpp: 45 pixbuf stream 0x5dd68664f480\n"
                     "                             Glib::RefPtr<Gdk::Pixbuf> WeatherImageRequest::get_pixbuf()\n"};
    auto fs = std::ofstream(logFile, std::ios::binary);
    if (fs) {
        fs.write(line.c_str(), line.length());
        fs.close();
        auto fileSize = std::filesystem::file_size(std::filesystem::path{logFile});
        auto logViewFile = psc::log::LogViewFile::create();
        std::map<psc::log::LogDays, uint64_t> mapDays;
        auto logViewId = std::make_shared<psc::log::LogViewIdFile>(psc::log::LogViewType::Name, ".", mapDays, fileSize, logFile);
        auto logViewTm = std::make_shared<psc::log::LogViewIdFile>(psc::log::LogViewType::Time, "2024-10-19");
        std::list<psc::log::pLogViewIdentifier> query;
        query.emplace_back(std::move(logViewId));
        query.emplace_back(std::move(logViewTm));
        logViewFile->setQuery(query);
        auto cnt = 0u;
        for (auto iter = logViewFile->begin(); iter != logViewFile->end(); ++iter) {
            auto entry = *iter;
            std::cout << "Level " << psc::log::Log::getLevel(entry.getLevel()) << "\n"
                  << " message " << entry.getMessage() << "\n"
                  << " time " << entry.getLocalTime()  << "\n"
                  << " location " << entry.getLocation() << std::endl;
            ++cnt;
        }
        std::remove(logFile.c_str());
        if (cnt != 1) {
            std::cout << "log_test_file expected cnt 1 was " << cnt << std::endl;
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}


int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Gio::init();   // includes Glib::init
    if (!test_log()) {
        return 1;
    }
    if (!test_logview()) {
        return 2;
    }
    if (!log_test_syslog()) {
        return 3;
    }
    if (!log_test_file()) {
        return 4;
    }
    if (!parse_test()) {
        return 5;
    }


    return 0;
}

