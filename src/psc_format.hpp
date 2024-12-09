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

#include <iostream>

// simplify format switching,
// alias them with out namespace


#if __GNUC__ >= 13
#include <format>
#else
// to make this work enable in genericImg the depend_libfmt definition and usages
#include <fmt/format.h>
#endif

namespace psc {
namespace fmt {

#if __GNUC__ >= 13
    using std::format;
    using std::formatter;
    using std::format_context;
    using std::vformat;
    using std::make_format_args;
#else
    using fmt::format;
    using fmt::formatter;
    using fmt::format_context;
    using fmt::vformat;
    using makefmt::make_format_args;
#endif

} /* namespace fmt */
} /* namespace psc */