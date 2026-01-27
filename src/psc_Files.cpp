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


#include "StringUtils.hpp"
#include "psc_Files.hpp"

namespace psc::util {

std::string psc_Files::getSrcRelativeDir(
              Glib::StdStringView execPath
            , Glib::StdStringView packageSrcDir
            , Glib::StdStringView relPath)
{
    //   build absolute executable path
    auto exec = Glib::canonicalize_filename(execPath, Glib::get_current_dir());
    auto execFile = Gio::File::create_for_path(exec);
    //   as PACKAGE_SRC_DIR is relative to executable resolve from path
    auto srcPath = Glib::canonicalize_filename(packageSrcDir, execFile->get_parent()->get_path());
    //   from src get sibling res
    auto resPath = Glib::canonicalize_filename(relPath, srcPath);
    return resPath;
}

std::string
psc_Files::getExtension(const Glib::RefPtr<Gio::File>& file)
{   // reference StringUtils as it was created there
    return StringUtils::getExtension(file);
}

std::string
psc_Files::getExtension(const std::string& filename)
{   // reference StringUtils as it was created there
    return StringUtils::getExtension(filename);
}

} // psc::util