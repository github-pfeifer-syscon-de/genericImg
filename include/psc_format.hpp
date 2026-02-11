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

#include <iostream>

// simplify format switching,
// alias them with our namespace


#if __cplusplus >= 202002L
#include <format>
#else
// to make this work enable in genericImg/configure.ac the depend_libfmt definition and usages (and install package)
#include <fmt/format.h>
#endif

namespace psc {

#if __cplusplus >= 202002L
namespace fmt {
    using std::format;
    using std::formatter;
    using std::format_context;
    using std::vformat;
    using std::make_format_args;
} /* namespace fmt */
#else
namespace {
     namespace fmt = fmt;
}
#endif

} /* namespace psc */
