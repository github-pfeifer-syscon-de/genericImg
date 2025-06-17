/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 rpf
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
#include <giomm-2.4/giomm.h>
#include <hpdf.h>
#include <memory>

namespace psc::pdf
{

class PdfExport;

class PdfFont
{
public:
    PdfFont(HPDF_Font font, std::string_view encoding);
    explicit PdfFont(const PdfFont& orig) = delete;
    virtual ~PdfFont() = default;

    HPDF_Font getPdfFont();
    // unmappable chars will be ignored
    // see https://github.com/libharu/libharu/blob/master/demo/encoding_list.c for viable encodings
    std::string encodeText(const Glib::ustring& us);

protected:

private:
    HPDF_Font m_font;
    std::string m_encoding;
    Glib::RefPtr<Gio::CharsetConverter> m_converter;
};

} /* end namespace psc::pdf */
