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

#include <gtkmm.h>
#include <hpdf.h>
#include <memory>
#include <string_view>
#include <exception>
#include <psc_format.hpp>

namespace psc::pdf
{

class PdfFormat
{
public:
    constexpr PdfFormat(float widthMm, float heightMm)
    : m_widthMm{widthMm}
    , m_heightMm{heightMm}
    {
    }
    PdfFormat(const PdfFormat& fmt) = default;
    virtual ~PdfFormat() = default;
    float getWidthMm()
    {
        return m_widthMm;
    }
    float getHeightMm()
    {
        return m_heightMm;
    }
private:
    float m_widthMm;
    float m_heightMm;
};


enum class Orientation
{
     Portrait
    ,Landscape
};

class PdfPage;
class PdfFont;

class PdfException
: public std::exception
{
public:
    PdfException(std::string_view message)
    : exception()
    , m_msg{message}
    {
    }
    virtual ~PdfException() = default;
    const char* what() const noexcept override
    {
        return m_msg.c_str();
    }
private:
    std::string m_msg;
};

class PdfExport
{
public:
    PdfExport();
    explicit PdfExport(const PdfExport& orig) = delete;
    virtual ~PdfExport();

    void save(const std::string& filename);
    HPDF_Doc getDoc();
    static float mm2dot(float mm);
    static float dot2mm(float dot);
    PdfFormat getFormat();
    void setFormat(PdfFormat pdfFormat);
    Orientation getOrientation();
    void setOrientation(Orientation orient);
    // use Base14 see names below
    std::shared_ptr<PdfFont> createFont(const Glib::ustring& fontName);
    // support dropped, requires to provided some type1 font
    //   in /usr/share/genericimg or as resource see findFontFile
    // this just cares about the font setting, ensure the text encoding is adjusted before passing to PdfPage::drawText
    //   PdfFont contains encodeText for this purpose,
    [[deprecated("requires type1 fonts downloading")]]
    std::shared_ptr<PdfFont> createFontInternalWithEncoding(std::string_view encoding);
    std::shared_ptr<PdfFont> createFontType1(const std::string& afm, const std::string& pfb, std::string_view encoding);
    // most useful with Css2 font-names e.g. "sans-serif"
    std::shared_ptr<PdfFont> createFontTTFMatch(const std::string& name, std::string_view encoding, bool embedd);
    std::shared_ptr<PdfFont> createFontTTF(const std::string& ttfName, std::string_view encoding, int idx, bool embedd);

    static constexpr auto BASE14FONT_COURIER{"Courier"};
    static constexpr auto BASE14FONT_COURIER_BOLD{"Courier-Bold"};
    static constexpr auto BASE14FONT_COURIER_OBLIQUE{"Courier-Oblique"};
    static constexpr auto BASE14FONT_COURIER_BOLDOB{"Courier-BoldOblique"};
    static constexpr auto BASE14FONT_HELVECTICA{"Helvetica"};
    static constexpr auto BASE14FONT_HELVECTICA_BOLD{"Helvetica-Bold"};
    static constexpr auto BASE14FONT_HELVECTICA_OBLIQUE{"Helvetica-Oblique"};
    static constexpr auto BASE14FONT_HELVECTICA_BOLDOB{"Helvetica-BoldOblique"};
    static constexpr auto BASE14FONT_TIMES{"Times-Roman"};
    static constexpr auto BASE14FONT_TIMES_BOLD{"Times-Bold"};
    static constexpr auto BASE14FONT_TIMES_ITALIC{"Times-Italic"};
    static constexpr auto BASE14FONT_TIMES_BOLDIT{"Times-BoldItalic"};
    static constexpr auto BASE14FONT_SYMBOL{"Symbol"};
    static constexpr auto BASE14FONT_ZAPFDINGSBAT{"ZapfDingbats"};

    static constexpr PdfFormat pdfFormatA6{105.f, 148.f};
    static constexpr PdfFormat pdfFormatA5{148.f, 210.f};
    static constexpr PdfFormat pdfFormatA4{210.f, 297.f};
    static constexpr PdfFormat pdfFormatA3{297.f, 420.f};
    static constexpr PdfFormat pdfFormatLetter{215.9f, 279.4f};

protected:
    Glib::RefPtr<Gio::File> findFontFile(const char* file);

private:
    HPDF_Doc m_pdf{nullptr};
    PdfFormat m_format{pdfFormatA4};
    Orientation m_orientation{Orientation::Portrait};
};

} /* end namespace psc::pdf */
