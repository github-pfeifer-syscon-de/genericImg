/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <type_traits>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <array>

#include "psc_format.hpp"
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
StringUtils::rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void
StringUtils::trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

Glib::ustring
StringUtils::lower(const Glib::ustring& str, int start)
{
    return Glib::ustring{str.substr(0, start) + str.substr(start).lowercase()};
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

std::vector<Glib::ustring> StringUtils::splitQuoted(const Glib::ustring &line, gunichar delim, gunichar quote)
{
    std::vector<Glib::ustring> ret;
    ret.reserve(16);
    bool inQuote{false};
    Glib::ustring part;
    part.reserve(64);
    for (Glib::ustring::size_type pos = 0; pos < line.length(); ++pos) {
        auto c = line.at(pos);
        bool isDelim{false};
        if (inQuote) {
            if (c == quote) {
                if (pos + 1 < line.length()
                 && line.at(pos + 1) == quote) {
                    ++pos;
                }
                else {
                    inQuote = false;
                    isDelim = true;
                }
            }
        }
        else {
            if (c == quote) {
                if (pos + 1 < line.length()
                 && line.at(pos + 1) == quote) {
                    ++pos;
                }
                else {
                    inQuote = true;
                    isDelim = true;
                }
            }
            else if (c == delim)  {
                isDelim = true;
            }
        }
        if (isDelim) {
            ret.push_back(part);
            part = "";
            while (pos+1 < line.length()) {
                c = line.at(pos+1);
                if (c != delim && c != quote) {
                    break;
                }
                if (c == quote) {
                    inQuote = !inQuote;
                }
                ++pos;
            }
        }
        else {
            part += c;
        }
    }
    if (!part.empty()) {
        ret.push_back(part);
    }
    return ret;
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
    psc::log::Log::logAdd(psc::log::Level::Warn, [&] {
        auto erc = std::make_error_code(ec);
        return psc::fmt::format("Formating {} failed {}", val, erc.message());
    });
    return "0";
}

Glib::ustring
StringUtils::typeName(const std::type_info& typeinfo)
{
#ifndef _MSC_VER
    return abi::__cxa_demangle(typeinfo.name(), nullptr, nullptr, nullptr);
#else
    return typeinfo.name();
#endif
}

std::string
StringUtils::hexdump(gchar* string, gsize size)
{
    std::string dump;
    dump.reserve(80 * ((size / HEXDUMP_LINE_SIZE) + 1));
    for (gsize l = 0; l < size; l += HEXDUMP_LINE_SIZE) {
        dump = psc::fmt::format("{:04x} : ", l);
        auto max = std::min(HEXDUMP_LINE_SIZE, size-l);
        for (gsize r = 0; r < HEXDUMP_LINE_SIZE; ++r) {
            if (r < max) {
                dump += psc::fmt::format("{:02x} ", string[l+r]);
            }
            else {
                dump += "   ";
            }
        }
        dump += "  ";
        for (gsize r = 0; r < max; ++r) {
            auto v= string[l+r];
            dump += psc::fmt::format("{:c}", v <= '}' ? (char)v : '.');
        }
        dump += '\n';
    }
    return dump;
}

Glib::ustring
StringUtils::getExtension(const Glib::RefPtr<Gio::File>& file)
{
    Glib::ustring base = file->get_basename();
    auto pos = base.find_last_of('.');
    if (pos != base.npos) {
        return base.substr(pos + 1);
    }
    return "";
}