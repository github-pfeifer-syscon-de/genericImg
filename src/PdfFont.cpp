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

#include <iostream>

#include "PdfFont.hpp"
#include "PdfExport.hpp"
#include "PdfPage.hpp"
#include "TableProperties.hpp"

namespace psc::pdf
{

PdfFontRef::PdfFontRef(HPDF_Font font)
: m_font{font}
{
}

HPDF_Font
PdfFontRef::getPdfFont()
{
    return m_font;
}

PdfFont::PdfFont(HPDF_Doc pdf, std::string_view detail_font_name, std::string_view encoding, float size)
: m_encoding{encoding}
, m_size{size}
{
    auto font = HPDF_GetFont(pdf, detail_font_name.data(), encoding.data());
    m_font = std::make_shared<PdfFontRef>(font);
}

PdfFont::PdfFont(std::shared_ptr<PdfFontRef> font, std::string_view encoding, float size)
: m_font{font}
, m_encoding{encoding}
, m_size{size}
{
}

HPDF_Font
PdfFont::getPdfFont()
{
    return m_font->getPdfFont();
}

std::shared_ptr<PdfFont>
PdfFont::derive(float size)
{
    return std::make_shared<PdfFont>(m_font, m_encoding, size);
}

std::string
PdfFont::encodeText(const Glib::ustring& us)
{
    if (m_encoding.empty() || m_encoding == "UTF-8") {
        return us;
    }
    if (!m_converter) {
        g_autoptr(GError) err{};
        //std::cout << "Creating encoder " << encoding << " in " << us.length() << " bytes " << us.bytes() << std::endl;
        // Gio::Charsetconvert doesn't do anything just freaks out ...
        GCharsetConverter* gconv = g_charset_converter_new(m_encoding.data(), "UTF-8", &err);
        if (err) {
            std::cout << "Error creating charset converter " << m_encoding << " " << err->message << std::endl;
            return us;
        }
        m_converter = Glib::wrap(gconv);
    }
    size_t size{us.bytes() + 16};   // expect single byte encodings, which reduce the size when converted, (but be careful if converter fallback gets selected)
    if (m_encoding == "UTF-16") {   // adapt this to encodings getting used
        size *= 2;
    }
    else if (m_encoding == "UTF-32") {
        size *= 4;
    }
    std::vector<char> buf(size);
    gsize read{},out{};
    try {
        m_converter->convert(
                  reinterpret_cast<const void*>(us.c_str()), us.bytes()
                , reinterpret_cast<void*>(&buf[0]), size
                , Gio::ConverterFlags::CONVERTER_NO_FLAGS, read, out);
    }
    catch (const Glib::Error& err) {
        std::cout << "Error " << err.what() << " charset " << m_encoding << " converting" << std::endl;
        return us;
    }
    //std::cout << "Read " << read << " out " << out << std::endl;
    //std::cout << "   to " << StringUtils::hexdump(&buf[0], out) << std::endl;
    std::string str(buf.data(), out);
    return str;
}


float
PdfFont::getTextWidth(std::shared_ptr<PdfPage> page, const Glib::ustring& us)
{
    auto pdfFont = m_font->getPdfFont();
    auto cs = page->getCharSpace();
    auto ws = page->getWordSpace();
    auto txt = encodeText(us);
    auto pdfBytes = reinterpret_cast<const HPDF_BYTE*>(txt.c_str());
    //std::cout << "cs " << cs << " ws " << ws << std::endl;
    float width;
    auto avail = HPDF_Font_MeasureText(pdfFont
                         , pdfBytes
                         , static_cast<HPDF_UINT>(txt.length())
                         , page->getWidth()               /* width */
                         , m_size                   /* font_size */
                         , cs                       /* char_space */
                         , ws                       /* word_space */
                         , HPDF_FALSE               /* wordwrap */
                         , &width);
    if (avail == 0) {
        width = 0.0f;
    }
    return width;
}

float
PdfFont::getCapHeight()
{
    auto pdfFont = m_font->getPdfFont();
    float capHeight = static_cast<float>(HPDF_Font_GetCapHeight(pdfFont));
    // see https://stackoverflow.com/questions/42320887/hpdf-units-for-text-width-and-height
    return capHeight / 1000.0f * m_size;   // 1000 suggested by doc for HPDF_Font_GetUnicodeWidth
}

float
PdfFont::getAscent()
{
    auto pdfFont = m_font->getPdfFont();
    float capHeight = static_cast<float>(HPDF_Font_GetAscent(pdfFont));
    // see https://stackoverflow.com/questions/42320887/hpdf-units-for-text-width-and-height
    return capHeight / 1000.0f * m_size;   // 1000 suggested by doc for HPDF_Font_GetUnicodeWidth
}


float
PdfFont::getDescent()
{
    auto pdfFont = m_font->getPdfFont();
    float descent = static_cast<float>(HPDF_Font_GetDescent(pdfFont));
    // see https://stackoverflow.com/questions/42320887/hpdf-units-for-text-width-and-height
    return descent / 1000.0f * m_size;   // 1000 suggested by doc for HPDF_Font_GetUnicodeWidth
}

float
PdfFont::getSize()
{
    return m_size;
}

void
PdfFont::setSize(float size)
{
    m_size = size;
}

float
PdfFont::getLeading()
{
    return getAscent() - getDescent();
}

} /* end namespace psc::pdf */


