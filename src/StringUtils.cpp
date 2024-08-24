/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2021 rpf
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

#include <locale>
#include <codecvt>
#include <algorithm>
#include <cctype>
#include <string>


#include "StringUtils.hpp"
#include "Log.hpp"

// assume input is utf8 as it may come from glib
//  wchar_t is even for 32bit plattform 32bits so we shoud better use utf32 for full compat fontconfig
//    this is the primary use for this function, but use utf16 for windows ?
//static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converterUtf8;
static std::wstring_convert<std::codecvt_utf8<wchar_t>> converterUtf8;
// for c++17 deprecated requires use local with facet, but for our purpose the above works fine

std::wstring
StringUtils::from_bytesUtf8(const char *in)
{
    return converterUtf8.from_bytes(in);
}

std::wstring
StringUtils::from_bytesUtf8(const std::string &in)
{
    return converterUtf8.from_bytes(in);
}

void
StringUtils::ltrim(Glib::ustring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](gunichar ch) {
        return !g_unichar_isspace(ch);
    }));
}

void
StringUtils::rtrim(Glib::ustring &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](gunichar ch) {
        return !g_unichar_isspace(ch);
    }).base(), s.end());
}

void
StringUtils::trim(Glib::ustring &s) {
    ltrim(s);
    rtrim(s);
}

void
StringUtils::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void
StringUtils::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void
StringUtils::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

Glib::ustring
StringUtils::lower(const Glib::ustring &str, int start) {
    int i = 0;
    Glib::ustring r;
    r.reserve(str.length());
    for (auto s = str.begin(); s != str.end(); ++s)
    {
        if (i >= start)
            r += g_unichar_tolower(*s);
        else
            r += *s;
        ++i;
    }
    return r;
}



const char *
StringUtils::weekday(int day)
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


void
StringUtils::split(const Glib::ustring &line, char delim, std::vector<Glib::ustring> &ret)
{
    size_t pos = 0;
    while (pos < line.length()) {
        size_t next = line.find(delim, pos);
        if (next != std::string::npos) {
            auto fld = line.substr(pos, next - pos);
            ret.push_back(fld);
            ++next;
        }
        else {
            if (pos < line.length()) {
                size_t end = line.length();
                if (line.at(end-1) == '\n') {
                    --end;
                }
                if (end - pos > 0) {
                    auto fld = line.substr(pos, end - pos);
                    ret.push_back(fld);
                }
            }
            break;
        }
        pos = next;
    }
}

void
StringUtils::splitRepeat(const Glib::ustring &line, gunichar delim, std::vector<Glib::ustring> &ret)
{
    size_t pos = 0;
    while (pos < line.length()) {
        while (line[pos] == delim
            && pos < line.length()) {
            ++pos;
        }
        if (pos < line.length()) {
            auto next = line.find(delim, pos);
            if (next != Glib::ustring::npos) {
                auto fld = line.substr(pos, next - pos);
                ret.push_back(fld);
            }
            else {
                if (pos < line.length()) {
                    size_t end = line.length();
                    if (line.at(end-1) == '\n') {
                        --end;
                    }
                    if (end - pos > 0) {
                        auto fld = line.substr(pos, end - pos);
                        ret.push_back(fld);
                    }
                }
                break;
            }
            pos = next;
        }
    }
}


Glib::ustring
StringUtils::replaceAll(const Glib::ustring& text, const Glib::ustring& replace, const Glib::ustring& with)
{
    Glib::ustring ret = text;
    size_t pos = 0;
    while ((pos = ret.find(replace, pos)) != Glib::ustring::npos) {
         ret.replace(pos, replace.length(), with);
         pos += with.length();
    }
    return ret;
}

#ifdef __WIN32__
// Convert a wide Unicode string to an UTF8 string windows style...
std::string
StringUtils::utf8_encode(const BSTR& bstr)
{
    if(bstr == nullptr)
        return std::string();
    auto len = static_cast<int>(wcslen(bstr));    // will cut at first null char.
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
#endif

bool
StringUtils::endsWith(const Glib::ustring& str, const Glib::ustring& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool
StringUtils::startsWith(const Glib::ustring& str, const Glib::ustring& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

std::ostream&
operator<< (std::ostream& os, const std::source_location& location)
{
    os << location.file_name() << "("
       << location.line() << ":"
       << location.column() << ") "
       << location.function_name() << ";";
    return os;
}

#if __cplusplus >= 202302L

std::ostream&
operator<< (std::ostream& os, const std::stacktrace& trace)
{
    for (auto iter = trace.begin(); iter != tract.end(); ++iter) {
        os << iter->source_file() <<
           << "(" << iter->source_line()
           << ")" << iter->description()
           << "\n";
    }
    return os;
}

#endif


double
StringUtils::parseCDouble(const Glib::ustring& val)
{
    double value = Glib::Ascii::strtod(val);
    return value;
}

Glib::ustring
StringUtils::formatCDouble(double val, std::chars_format fmt, int precision)
{
    // just some alternative:
    // auto spi = Glib::Ascii::dtostr(pi); // this creates many decimal places (for some cases e.g. 3.14)
    std::array<char, 64> str;
    auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), val, fmt, precision);
    if (ec == std::errc()) {
        Glib::ustring ustr{str.data(), ptr};
        return ustr;
    }
    psc::log::Log::logAdd(psc::log::Level::Warn, Glib::ustring::sprintf("Formating %lf failed ", val));
    return "0";
}