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

#include <glibmm.h>

namespace psc {
namespace utl {


class DateUtils
{
public:
    DateUtils() = delete;
    explicit DateUtils(const DateUtils& orig) = delete;
    virtual ~DateUtils() = delete;

    // a simplified DateTime parse
    //  only b d Y T F formats are supported
    //  separations chars are needed between fields
    static Glib::DateTime parse(const char* fmt, const std::string& val, const Glib::TimeZone& tz = Glib::TimeZone::create_utc());
    static constexpr std::array<const char*,12> months_en {"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
    static bool starts_with_ignore_case(const Glib::ustring& stack, const Glib::ustring& part);
    static const char *weekday(int day);

private:

};

} /* namespace utl */
} /* namespace psc */

