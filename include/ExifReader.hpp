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

#include <gtkmm.h>

class ImageList;

class ExifReader {
public:
    ExifReader(ImageList& imageList);
    virtual ~ExifReader() = default;

    void decode(const Glib::ustring& exifStr);
    void append2IdfList(const char *name, Glib::ustring& value);
    void append2ExifList(const char *ifdName);

protected:
    uint8_t* toBuffer(const Glib::ustring& exifStr, uint32_t &retLen);

    ImageList& m_imageList;
    Gtk::TreeIter m_iterExif;
    Gtk::TreeIter m_chldsIdf;
private:
};

