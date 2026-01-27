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


#include <glibmm.h>
#include <string_view>

#include "ApplicationSupport.hpp"

namespace psc::util {

class Files {
public:
    /**
     * getSrcRelativeDir
     *   resolve the relative PACKAGE_SRC_DIR (given relative to executable) as an absolute path
     *   expects the current working directory unchanged from start
     * @Param execPath set as argv[0]
     * @Param packageSrcDir set as macro PACKAGE_SRC_DIR
     * @Param relPath defaults to "../res"
     * @return absolute path
     **/
    static std::string getSrcRelativeDir(
              Glib::StdStringView execPath /* argv[0] */
            , Glib::StdStringView packageSrcDir /* use with PACKAGE_SRC_DIR */
            , Glib::StdStringView relPath = "../res");

    // extension without "." e.g. xz.cpp -> cpp
    static std::string getExtension(const Glib::RefPtr<Gio::File>& file);
    static std::string getExtension(const std::string& filename);
};

} // namespace psc::util
