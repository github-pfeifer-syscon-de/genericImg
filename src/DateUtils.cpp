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
#include <string>
#include <cstring>
#include <charconv>

#include "DateUtils.hpp"

namespace psc {
namespace utl {


bool
DateUtils::starts_with_ignore_case(const Glib::ustring& stack, const Glib::ustring& part)
{
    if (stack.length() < part.length()) {
        return false;
    }
    for (size_t i = 0; i < part.length(); ++i) {
        if (Glib::Unicode::tolower(stack[i]) != Glib::Unicode::tolower(part[i])) {
            return false;
        }
    }
    return true;
}


static Glib::Date::Month
parseMonthAbbr(const std::string& token)
{
    int mi = 0;
    for (size_t i = 0; i < DateUtils::months_en.size(); ++i) {
        if (DateUtils::starts_with_ignore_case(token, DateUtils::months_en[i])) {
            mi = static_cast<int>(i) + Glib::Date::JANUARY;
            break;
        }
    }
    if (mi == 0) {  // alternative try locale, as it takes more effort (but beware the english abbr. match the wrong month in your language...)
        for (int m = Glib::Date::JANUARY; m <= Glib::Date::DECEMBER; ++m){
            auto dt = Glib::Date((Glib::Date::Day)1, (Glib::Date::Month)1, (Glib::Date::Year)2024);
            dt.set_month(static_cast<Glib::Date::Month>(m));
            auto month = dt.format_string("%b");
            if (DateUtils::starts_with_ignore_case(token, month)) {
                mi = m;
                break;
            }
        }

    }
    auto m = static_cast<Glib::Date::Month>(mi);
    //std::cout << "parseMonth " << token << " m " << mi << std::endl;
    return m;
}

static Glib::Date::Day
parseDay(const std::string& token)
{
    auto d = static_cast<Glib::Date::Day>(std::stoi(token));
    //std::cout << "parseDay " << token << " d " << (int)d << std::endl;
    return d;
}

static Glib::Date::Year
parseYear(const std::string& token)
{
    auto y = static_cast<Glib::Date::Year>(std::stoi(token));
    //std::cout << "parseYear " << token << " y " << y << std::endl;
    return y;
}

static bool
parseTime(const std::string& token, int& hour, int& minute, double& second)
{
    size_t pos = 0;
    size_t end = token.find(':', pos);
    if (end == token.npos) {
        return false;
    }
    hour = std::stoi(token.substr(pos, end));
    pos = end + 1;
    end = token.find(':', pos);
    if (end == token.npos) {
        return false;
    }
    minute = std::stoi(token.substr(pos, end));
    pos = end + 1;
    if (pos < token.length()) {
        auto sec = token.substr(pos);   // use from_chars as we don't want local specific parsing
        auto [ptr, ec] = std::from_chars(sec.data(), sec.data() + sec.length(), second);
        if (ec != std::errc()) {
            second = std::stoi(sec);    // fallback to integer
        }
    }
    else {
        second = 0;
    }
    //std::cout << "parseTime " << token << " h " << hour << " m " << minute << " s " << second << std::endl;
    return true;
}


static bool
parseIso(const std::string& token, Glib::Date::Year& year, Glib::Date::Month& month, Glib::Date::Day& day)
{
    size_t pos = 0;
    size_t end = token.find('-', pos);
    if (end == token.npos) {
        return false;
    }
    year = static_cast<Glib::Date::Year>(std::stoi(token.substr(pos, end)));
    pos = end + 1;
    end = token.find('-', pos);
    if (end == token.npos) {
        return false;
    }
    month = static_cast<Glib::Date::Month>(std::stoi(token.substr(pos, end)));
    pos = end + 1;
    if (pos < token.length()) {
        day = static_cast<Glib::Date::Day>(std::stoi(token.substr(pos)));
        return true;
    }
    else {
        day = 1;
    }
    //std::cout << "parseTime " << token << " y " << year << " m " << month << " d " << day << std::endl;
    return false;
}


Glib::DateTime
DateUtils::parse(const char* fmt, const std::string& val, const Glib::TimeZone& tz)
{
    size_t pos{0};
    Glib::Date::Year year{1970};
    Glib::Date::Month month(Glib::Date::JANUARY);
    Glib::Date::Day day(1);
    int hour{0};
    int minute{0};
    double second{0.0};
    const size_t len = std::strlen(fmt);
    for (size_t i = 0; i < len; ++i) {
        char c = fmt[i];
        if (c == '%') {
            ++i;
            if (i >= len) {
                break;
            }
            c = fmt[i];
            ++i;
            size_t end = val.length();
            std::string token;
            if (i < len) {
                char delim = fmt[i];
                //std::cout << "using delim \"" << delim << "\"" << " pos " << pos << std::endl;
                end = val.find(delim, pos);
                if (end == val.npos) {
                    break;
                }
            }
            token = val.substr(pos, end-pos);
            //std::cout << "token " << token << " pos " << pos << " end " << end << std::endl;
            switch (c) {
            case 'b':   // month abbr.
                month = parseMonthAbbr(token);
                break;
            case 'd':   // day of month
                day = parseDay(token);
                break;
            case 'Y':   // year
                year = parseYear(token);
                break;
            case 'T':   // time h:m:s
                parseTime(token, hour, minute, second);
                break;
            case 'F':   // iso-date y-m-d
                parseIso(token, year, month, day);
                break;
            }
            pos = end + 1;
        }
    }
    Glib::DateTime dateTime = Glib::DateTime::create(tz, year, month, day, hour, minute, second);
    return dateTime;
}

const char *
DateUtils::weekday(int day)
{
    switch (day%7)
    {
        case 0:
            return "Sun";
        case 1:
            return "Mon";
        case 2:
            return "Tue";
        case 3:
            return "Wed";
        case 4:
            return "Thu";
        case 5:
            return "Fri";
        case 6:
            return "Sat";
    }
    return "?";
}

} /* namespace utl */
} /* namespace psc */