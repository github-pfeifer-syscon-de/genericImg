/*
 * Copyright (C) 2025 RPf <gpl3@pfeifer-syscon.de>
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


#include <cstdlib>
#include <iostream>
#include <StringUtils.hpp>
#include <glibmm.h>

#include "PdfExport.hpp"
#include "PdfPage.hpp"


static std::string
encode(const Glib::ustring& us, const char* encoding)
{
    g_autoptr(GError) err{};
    //std::cout << "Creating encoder " << encoding << " in " << us.length() << " bytes " << us.bytes() << std::endl;
    // Gio::Charsetconvert doesn't do anything just freaks out ...
    GCharsetConverter* gconv = g_charset_converter_new(encoding, "UTF-8", &err);
    if (err) {
        std::cout << "Error creating charset converter " << encoding << " " << err->message << std::endl;
        return "";
    }
    auto conv = Glib::wrap(gconv);
    size_t size{us.bytes() + 16};   // expect single byte encodings, which reduce the size when converted
    std::vector<char> buf(size);
    gsize read{},out{};
    Gio::ConverterResult res = conv->convert(
              reinterpret_cast<const void*>(us.c_str()), us.bytes()
            , reinterpret_cast<void*>(&buf[0]), size
            , Gio::ConverterFlags::CONVERTER_NO_FLAGS, read, out);// GConverterFlags::G_CONVERTER_NO_FLAGS
    if (res == Gio::ConverterResult::CONVERTER_ERROR) {
        std::cout << "Error " << static_cast<int>(res) << " charset " << encoding << " converting" << std::endl;
        out = 0;
    }
    //std::cout << "Read " << read << " out " << out << std::endl;
    //std::cout << "   to " << StringUtils::hexdump(&buf[0], out) << std::endl;
    std::string str(buf.data(), out);
    return str;
}

// check the create file test.pdf
static bool
pdf_test()
{
    auto PdfEncoding{"ISO8859-15"};
    auto pdfExport{std::make_shared<PdfExport>()};
    pdfExport->setEncoding(PdfEncoding);
    auto font = pdfExport->createFont();
    auto page = std::make_shared<PdfPage>(pdfExport);
    page->setFont(font, 12.0f);
    Glib::ustring us = StringUtils::u8str(u8"abc öäüÖÄÜß°");
    std::string etext = encode(us, PdfEncoding);
    page->drawText("iso " + etext, 40, page->getHeight() - 40);
    //page->drawText("utf " + us, 40, page->getHeight() - 60);

    pdfExport->save("test.pdf");
    return true;
}
/*
 *
 */
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Glib::init();
    Gio::init();    // need for Gio::CharsetConverter
    //example();
    if (!pdf_test()) {
        return 1;
    }

    return 0;

}

