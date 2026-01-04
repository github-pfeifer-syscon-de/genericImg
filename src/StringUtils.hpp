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


#pragma once

#include <string>
#include <vector>
#ifdef __WIN32__
#include <windows.h>
#include <wchar.h>
#endif
#include <source_location>
#include <iostream>
#include <charconv>
#include <typeinfo>
#include <glibmm.h>
#include <giomm.h>
#include <cstdint>

#include "psc_format.hpp"

class StringUtils {
public:
    virtual ~StringUtils() = default;

    // trim from start (in place)
    static void ltrim(Glib::ustring &s);

    // trim from end (in place)
    static void rtrim(Glib::ustring &s);

    // trim from both ends (in place)
    static void trim(Glib::ustring &s);

    static void ltrim(std::string &s);
    static void rtrim(std::string &s);
    static void trim(std::string &s);
    [[deprecated("see lower(size_t)")]]
    static Glib::ustring lower(const Glib::ustring &s, int start = 0);
    static inline Glib::ustring lower(const Glib::ustring &str, size_t start = 0)
    {
        return Glib::ustring{str.substr(0, start) + str.substr(start).lowercase()};
    }
    static std::string lowerStd(const std::string &str, size_t start = 0);

    static std::wstring from_bytesUtf8(const char *in);
    static std::wstring from_bytesUtf8(const std::string &in);

    static void split(const Glib::ustring &line, char delim, std::vector<Glib::ustring> &ret);
    // like split but works with repeated delimiters
    static void splitRepeat(const Glib::ustring &line, gunichar delim, std::vector<Glib::ustring> &ret);
    // split but consider text in quotes as no split
    //   as quotes are removed from the resulting string
    //   you need to put quote+quote to get one quote in output
    static std::vector<Glib::ustring> splitQuoted(const Glib::ustring &line, gunichar delim = ' ', gunichar quote = '"');

    static Glib::ustring replaceAll(const Glib::ustring& text, const Glib::ustring& replace, const Glib::ustring& with);
   // simple fix for the ustring <-> char8_t incompatibility
    static inline Glib::ustring u8str(const char8_t* cnst) {
        return Glib::ustring(reinterpret_cast<const char*>(cnst));
    }
    #ifdef __WIN32__
    // Convert a wide Unicode string to an UTF8 string
    static std::string utf8_encode(const BSTR& wstr);
    #endif
    static bool endsWith(const std::string& str, const std::string& suffix);
    static bool endsWith(const Glib::ustring& str, const Glib::ustring& suffix);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool startsWith(const Glib::ustring& str, const Glib::ustring& prefix);
    // improved conversion functions that do not depend on LocaleContext "magic"
    static double parseCDouble(const Glib::ustring& sval);
    // std::chars_format::general formats 3.14 to "3.14"
    // std::chars_format::fixed formats 3.14 to "3.1400" (precision 4)
    static Glib::ustring formatCDouble(double val, std::chars_format fmt = std::chars_format::general, int precision = 4);

    // type to readable name
    static Glib::ustring typeName(const std::type_info& typeinfo);
    static constexpr gsize HEXDUMP_SIZE{16};

    template<typename T
            , std::enable_if_t
                <    std::is_integral_v<T>
                  || std::is_void_v<T>
                  , bool
                > = true
            >
    static std::string hexdump(T* string, gsize size)
    {
        std::string dump;
        gsize lineSize = HEXDUMP_SIZE / sizeof(T);
        dump.reserve(80 * ((size * sizeof(T) / HEXDUMP_SIZE) + 1));
        auto fmt = psc::fmt::format("{{:0{}x}} ", sizeof(T) * 2);
        for (gsize l = 0; l < size; l += lineSize) {
            dump += psc::fmt::format("{:04x} : ", l);
            auto max = std::min(lineSize, size-l);
            for (gsize r = 0; r < max; ++r) {
                auto v = std::make_unsigned_t<T>(string[l+r]);
                dump += psc::fmt::vformat(fmt, std::make_format_args(v));
            }
            auto rem = lineSize - max;
            dump += std::string(((sizeof(T) * 2 + 1) * rem) + 4, ' ');
            for (gsize r = 0; r < max; ++r) {
                auto v = std::make_unsigned_t<T>(string[l+r]);
                dump += psc::fmt::format("{:c}", v >= ' ' &&  v <= '~' ? (char)v : '.');
            }
            dump += '\n';
        }
        return dump;
    }
    template <typename T, typename C>
    static C concat(const std::vector<T>& parts, C seperator, std::function<C(const T& t)>& func)
    {
        C out;
        size_t resv{1 + seperator.length() * parts.size()};
        for (auto& part : parts) {
            resv += func(part).length();
        }
        out.reserve(resv);
        for (auto& part : parts) {
            if (out.length() > 0) {
                out += seperator;
            }
            out += func(part);
        }
        return out;
    }
    template <typename T, typename C>
    static std::vector<T> splitEach(const T& line, const C c)
    {
        std::function<size_t(const T&, size_t, size_t&)> lambdaFind =
        [c] (const T& fline, size_t pos, size_t& next) -> auto
        {
            auto found = fline.find(c, pos);
            if (found != T::npos) {
                next = found + 1;
            }
            return found;
        };
        return splitFunc(line, lambdaFind);
    };
    template <typename T, typename C>
    static std::vector<T> splitConsec(const T& line, const C c)
    {
        std::function<size_t(const T&, size_t, size_t&)> lambdaFind =
        [c] (const T& fline, size_t pos, size_t& next) -> auto
        {
            auto found = fline.find(c, pos);
            if (found != T::npos) {
                next = found;
                while (next < fline.length() && fline[next] == c) {
                    ++next;
                }
            }
            return found;
        };
        return splitFunc(line, lambdaFind);
    };
    template <typename T>
    static std::vector<T> splitFunc(const T &line, std::function<size_t(const T& t, size_t pos, size_t& next)>& func)
    {
        std::vector<T> ret;
        size_t pos{};
        size_t resv{};
        while (pos < line.length()) {
            size_t next;
            size_t found = func(line, pos, next);
            ++resv;
            if (found == std::string::npos) {
                break;
            }
            pos = next;
        }
        if (resv > 0) {
            ret.reserve(resv);
            pos = 0;
            while (pos < line.length()) {
                size_t next;
                size_t found = func(line, pos, next);
                if (found != std::string::npos) {
                    auto fld = line.substr(pos, found - pos);
                    ret.push_back(fld);
                    pos = next;
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
        return ret;
    }

    // extension without "." e.g. xz.cpp -> cpp
    static std::string getExtension(const Glib::RefPtr<Gio::File>& file);
    static std::string getExtension(const std::string& filename);
protected:
    static constexpr uint64_t HEXDUMP_LINE_SIZE{16};
private:
    StringUtils() = default;

};

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

// does not fit exactly, but no need for separate header
//  see Peter Muldoon https://www.youtube.com/watch?v=HXJmrMnnDYQ&t=2795s
std::ostream&
operator<< (std::ostream& os, const std::source_location& location);

#if __cplusplus >= 202302L
#include <stacktrace>

std::ostream&
operator<< (std::ostream& os, const std::stacktrace& trace);

#endif

template <>
struct psc::fmt::formatter<Glib::ustring>
: psc::fmt::formatter<std::string>
{
    auto format(const Glib::ustring& obj, psc::fmt::format_context& ctx) const
    {
        return psc::fmt::formatter<std::string>::format(obj, ctx);
    }
};

