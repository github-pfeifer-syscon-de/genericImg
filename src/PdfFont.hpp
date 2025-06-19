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
class PdfPage;

class PdfFontRef
{
public:
    PdfFontRef(HPDF_Font font);
    explicit PdfFontRef(const PdfFontRef& orig) = delete;
    virtual ~PdfFontRef() = default;
    HPDF_Font getPdfFont();
private:
    HPDF_Font m_font;
};

class PdfFont
{
public:
    PdfFont(HPDF_Doc pdf, std::string_view detail_font_name, std::string_view encoding, float size = 12.0f);
    PdfFont(std::shared_ptr<PdfFontRef> font, std::string_view encoding, float size = 12.0f);
    explicit PdfFont(const PdfFont& orig) = delete;
    virtual ~PdfFont() = default;

    HPDF_Font getPdfFont();
    std::shared_ptr<PdfFont> derive(float size);
    // unmappable chars will be ignored
    // see https://github.com/libharu/libharu/blob/master/demo/encoding_list.c for viable encodings
    std::string encodeText(const Glib::ustring& us);
    float getTextWidth(std::shared_ptr<PdfPage> page, const Glib::ustring& us);
    float getSize();
    void  setSize(float size);
    float getLeading();
    float getDescent();
    float getCapHeight();
    float getAscent();



protected:

private:
    std::shared_ptr<PdfFontRef> m_font;
    std::string m_encoding;
    float m_size;
    Glib::RefPtr<Gio::CharsetConverter> m_converter;
};

} /* end namespace psc::pdf */
