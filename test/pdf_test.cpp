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
#include "PdfFont.hpp"



// check the create file test.pdf
static bool
pdf_test()
{
    auto PdfEncoding{"ISO8859-15"};
    auto pdfExport{std::make_shared<psc::pdf::PdfExport>()};
    auto font = pdfExport->createFontInternalWithEncoding(PdfEncoding);
    auto page = std::make_shared<psc::pdf::PdfPage>(pdfExport);
    page->setFont(font, 12.0f);
    Glib::ustring us = StringUtils::u8str(u8"abc öäüÖÄÜß°");
    std::string etext = font->encodeText(us);
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

