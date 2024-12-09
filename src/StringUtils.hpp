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
#include <glibmm.h>
#include <typeinfo>

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

    static std::wstring from_bytesUtf8(const char *in);
    static std::wstring from_bytesUtf8(const std::string &in);

    static void split(const Glib::ustring &line, char delim, std::vector<Glib::ustring> &ret);
    // like split but works with repeated delimiters
    static void splitRepeat(const Glib::ustring &line, gunichar delim, std::vector<Glib::ustring> &ret);

    static Glib::ustring replaceAll(const Glib::ustring& text, const Glib::ustring& replace, const Glib::ustring& with);
   // simple fix for the ustring <-> char8_t incompatibility
    static inline Glib::ustring u8str(const char8_t* cnst) {
        return Glib::ustring(reinterpret_cast<const char*>(cnst));
    }
    #ifdef __WIN32__
    // Convert a wide Unicode string to an UTF8 string
    static std::string utf8_encode(const BSTR& wstr);
    #endif
    static bool endsWith(const Glib::ustring& str, const Glib::ustring& suffix);
    static bool startsWith(const Glib::ustring& str, const Glib::ustring& prefix);
    // improved conversion functions that do not depend on LocaleContext "magic"
    static double parseCDouble(const Glib::ustring& sval);
    // std::chars_format::general formats 3.14 to "3.14"
    // std::chars_format::fixed formats 3.14 to "3.1400" (precision 4)
    static Glib::ustring formatCDouble(double val, std::chars_format fmt = std::chars_format::general, int precision = 4);

    // type to readable name
    static Glib::ustring typeName(const std::type_info& typeinfo);
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

