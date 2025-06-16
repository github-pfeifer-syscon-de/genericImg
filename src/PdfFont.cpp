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
#include "TableProperties.hpp"

namespace psc::pdf
{

PdfFont::PdfFont(HPDF_Font font, std::string_view encoding)
: m_font{font}
, m_encoding{encoding}
{
}

HPDF_Font
PdfFont::getPdfFont()
{
    return m_font;
}

std::string
PdfFont::encodeText(const Glib::ustring& us)
{
    if (!m_converter && !m_encoding.empty()) {
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
    size_t size{us.bytes() + 16};   // expect single byte encodings, which reduce the size when converted, (but be careful if fallback gets selected)
    std::vector<char> buf(size);
    gsize read{},out{};
    try {
        Gio::ConverterResult res = m_converter->convert(
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

} /* end namespace psc::pdf */


