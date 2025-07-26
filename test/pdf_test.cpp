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
#include "PdfImage.hpp"

// check the create pdf-files


// as there were some thoughts about distributing type1 fonts
//   dropped support, but if your are curious see res/Makefile.am
static bool
pdf_test()
{
//    auto PdfEncoding{"ISO8859-15"};
//    auto pdfExport{std::make_shared<psc::pdf::PdfExport>()};
//    auto font = pdfExport->createFontInternalWithEncoding(PdfEncoding);
//    auto page = std::make_shared<psc::pdf::PdfPage>(pdfExport);
//    page->setFont(font, 14.0f);
//    Glib::ustring us = StringUtils::u8str(u8"iso abc öäüÖÄÜß°☺");
//    page->drawText(us, 40, page->getHeight() - 40);
//    //page->drawText("utf " + us, 40, page->getHeight() - 60);
//
//    pdfExport->save("test.pdf");
    return true;
}

static float cellWidth;
static float cellHeight;
static float gridHeight;

static constexpr auto X_MARGIN{40.0f};
static constexpr auto Y_MARGIN{40.0f};

static float
getGridX(int32_t x)
{
    return x * cellWidth + X_MARGIN;
}

static float
getGridY(int32_t y)
{
    return gridHeight - y * cellHeight;
}



static void centered(const std::shared_ptr<psc::pdf::PdfPage>& page,
                     const std::shared_ptr<psc::pdf::PdfFont>& font,
                     const Glib::ustring& txt, float x, float y)
{
    float capHeight = font->getCapHeight();
    float descent = font->getDescent();
    float width = page->getTextWidth(txt);
    page->drawText(txt, x + (cellWidth - width) / 2.0f, y - (cellHeight - capHeight - descent) / 2.0f );
}



static void
drawGrid(const std::shared_ptr<psc::pdf::PdfPage>& page
       , const std::shared_ptr<psc::pdf::PdfFont>& font)
{
    page->setLineWidth(0.5f);
    page->setFont(font);
    auto pageHeigh = page->getHeight();
    auto pageWidth = page->getWidth();
    cellWidth = (pageWidth  - 2.0f * X_MARGIN) / 17.0f;
    cellHeight = (pageHeigh - 2.0f * Y_MARGIN) / 17.0f;
    gridHeight = Y_MARGIN + 17.0f * cellHeight;
    auto gridWidth = X_MARGIN + 17.0f * cellWidth;
    for (int32_t i = 0; i <= 17; i++) {
        float x = getGridX(i);
        float y = getGridY(0);
        page->moveTo(x, gridHeight);
        page->lineTo(x, Y_MARGIN);
        page->stroke();
        if (i > 0 && i <= 16) {
            auto txt = Glib::ustring::sprintf("x%X", i - 1);
            centered(page, font, txt, x ,y);
        }
    }

    for (int32_t i = 0; i <= 17; i++) {
        float x = getGridX(0);
        float y = getGridY(i);
        page->moveTo(x, y);
        page->lineTo(gridWidth, y);
        page->stroke();
        if (i > 0 && i <= 16) {
            auto txt = Glib::ustring::sprintf("%Xx", i - 1);
            centered(page, font,txt, x, y);
        }
    }
}

static void
drawChars(const std::shared_ptr<psc::pdf::PdfPage>& page
        , const std::shared_ptr<psc::pdf::PdfFont>& font
        , int upage)
{
    page->setFont(font);
    auto head = Glib::ustring::sprintf("%Xxx", upage);
    float x = getGridX(0);
    float y = getGridY(0);
    centered(page, font, head, x, y);
    for (int32_t i = 1; i < 17; i++) {
        for (int32_t j = 1; j < 17; j++) {
            gunichar c = upage * 256 + (i - 1) * 16 + (j - 1);
            if (c >= 32) {
                x = getGridX(j);
                y = getGridY(i);
                auto txt = Glib::ustring::sprintf("%lc", c);
                centered(page, font, txt, x, y);
            }
        }
    }
}

static bool
ttf_test()
{
    auto pdfEncoding{"UTF-8"};
    auto pdfExport{std::make_shared<psc::pdf::PdfExport>()};
    auto font = pdfExport->createFontTTFMatch("sans-serif", pdfEncoding, false);
    auto page1 = pdfExport->createPage();

    std::cout << "size " << font->getSize() << std::endl;
    std::cout << "   ascent " << font->getAscent() << std::endl;
    std::cout << "   capHeight " << font->getCapHeight() << std::endl;
    std::cout << "   descent " << font->getDescent() << std::endl;
    std::cout << "   leading " << font->getLeading() << std::endl;

    auto font14 = font->derive(14.0f);
    drawGrid(page1, font);
    drawChars(page1, font14, 0);
    auto page2 = pdfExport->createPage();
    drawGrid(page2, font);
    drawChars(page2, font14, 1);
    auto page3 = pdfExport->createPage();
    std::vector<Glib::ustring> lines;
    lines.push_back("0: aaaaaaaaäää");
    lines.push_back("1: bbbbbbbbüüü");
    lines.push_back("2: ccccccccööö");
    lines.push_back("3: ddddddddßßß");
    page3->setFont(font);
    page3->setTextPos(X_MARGIN, gridHeight);
    page3->drawTextLines(lines);
    lines.clear();
    lines.push_back("4: eeeeeeeÄÄÄÄ");
    lines.push_back("5: fffffffÜÜÜÜ");
    page3->setFont(font14);
    page3->setTextPos(X_MARGIN, gridHeight - 60.0f);
    page3->drawTextLines(lines);

    pdfExport->save("test_ttf.pdf");
    return true;
}

static bool
image_test(int size)
{
    auto pdfExport{std::make_shared<psc::pdf::PdfExport>()};
    auto image = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, size, size);
    auto ctx = Cairo::Context::create(image);
    ctx->set_antialias(Cairo::Antialias::ANTIALIAS_GRAY);
    ctx->set_source_rgb(0.0, 0.0, 0.0);
    ctx->rectangle(0.0, 0.0, image->get_width(), image->get_height());
    ctx->fill();
    ctx->set_line_width(2.0);
    ctx->set_source_rgb(1.0, 1.0, 1.0);
    ctx->move_to(0.0, 0.0);
    ctx->line_to(image->get_width(), image->get_height());
    ctx->stroke();
    ctx->select_font_face("sans-serif", Cairo::FontSlant::FONT_SLANT_NORMAL, Cairo::FontWeight::FONT_WEIGHT_NORMAL);
    ctx->set_font_size(12.0);
    ctx->move_to(2.0, 20.0);
    ctx->show_text("abcdef");

    auto pdfImage =std::make_shared<psc::pdf::PdfImage>(pdfExport);
    pdfImage->load(image);
    auto page = pdfExport->createPage();
    page->drawImage(pdfImage, 10.0f, page->getHeight() - 210.0f, 200.0f, 200.0f);

    pdfExport->save("test_img.pdf");
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
    if (!ttf_test()) {
        return 2;
    }
    if (!image_test(200)) {
        return 3;
    }

    return 0;

}

