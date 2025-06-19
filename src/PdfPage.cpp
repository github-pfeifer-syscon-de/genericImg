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

#include <StringUtils.hpp>

#include "PdfPage.hpp"
#include "PdfExport.hpp"
#include "PdfFont.hpp"
#include "PdfImage.hpp"

namespace psc::pdf {

PdfPage::PdfPage(std::shared_ptr<PdfExport> pdfExport)
: m_pdfExport{pdfExport}
{
    HPDF_Doc pdf = m_pdfExport->getDoc();
    /* add a new page object. */
    m_page = HPDF_AddPage(pdf);
    PdfFormat fmt = m_pdfExport->getFormat();
    Orientation orient = m_pdfExport->getOrientation();
    setFormat(fmt, orient);
}


void
PdfPage::setFormat(PdfFormat fmt, Orientation orient)
{
    float width = PdfExport::mm2dot(fmt.getWidthMm());
    float height = PdfExport::mm2dot(fmt.getHeightMm());
    if (orient == Orientation::Landscape) {
        std::swap(width, height);
    }
    HPDF_Page_SetWidth(m_page, width);
    HPDF_Page_SetHeight(m_page, height);

    m_dst = HPDF_Page_CreateDestination(m_page);
    HPDF_Destination_SetXYZ(m_dst, 0, HPDF_Page_GetHeight(m_page), 1);
    HPDF_Doc pdf = m_pdfExport->getDoc();
    HPDF_SetOpenAction(pdf, m_dst);
}

void
PdfPage::getFormat(float& widthMM, float& heightMM)
{
    auto dotWith = HPDF_Page_GetWidth(m_page);
    auto dotHeight = HPDF_Page_GetHeight(m_page);
    widthMM = PdfExport::dot2mm(dotWith);
    heightMM = PdfExport::dot2mm(dotHeight);
}

float
PdfPage::getWordSpace()
{
    return HPDF_Page_GetWordSpace(m_page);
}

float
PdfPage::getCharSpace()
{
    return HPDF_Page_GetCharSpace(m_page);
}

float
PdfPage::getHeight()
{
    return HPDF_Page_GetHeight(m_page);
}

float
PdfPage::getWidth()
{
    return HPDF_Page_GetWidth(m_page);
}

void
PdfPage::drawImage(std::shared_ptr<PdfImage>& image, float x, float y, float width, float height)
{
    /* Draw image to the canvas. */
    HPDF_Image img = image->getPdfImage();
    if (img) {
        HPDF_Page_DrawImage(m_page, img
                            , x, y
                            , width, height);
    }
    else {
        std::cout << "PdfPage::drawPng no image contained! (require load first)" << std::endl;
    }
}

void
PdfPage::drawText(const Glib::ustring& text, const std::shared_ptr<PdfFont>& font, float x, float y)
{
    std::string encoded = font->encodeText(text);
    drawText(encoded, font, x, y);
}

void
PdfPage::drawText(const std::string& text, const std::shared_ptr<PdfFont>& font, float x, float y)
{
    HPDF_Page_BeginText(m_page);
    HPDF_Page_SetFontAndSize(m_page, font->getPdfFont(), font->getSize());
    HPDF_Page_SetTextLeading(m_page, font->getLeading());
    HPDF_Page_MoveTextPos(m_page, x, y);
    std::vector<std::string> lines = StringUtils::splitEach(text, '\n');
    for (auto line : lines) {
        HPDF_Page_ShowTextNextLine(m_page, line.c_str());
    }
    HPDF_Page_EndText(m_page);
}

void
PdfPage::setRgb(float r, float g, float b)
{
    HPDF_Page_SetRGBFill(m_page, r, g, b);
    HPDF_Page_SetRGBStroke(m_page, r, g, b);
}

void
PdfPage::fill()
{
    HPDF_Page_Fill(m_page);
}

void
PdfPage::setLineWidth(float line_width)
{
    HPDF_Page_SetLineWidth(m_page, line_width);
}

void
PdfPage::moveTo(float x, float y)
{
    HPDF_Page_MoveTo(m_page, x, y);
}

void
PdfPage::lineTo(float x, float y)
{
    HPDF_Page_LineTo(m_page, x, y);
}

void
PdfPage::circle(float x, float y, float r)
{
    HPDF_Page_Circle(m_page, x, y, r);
}

void
PdfPage::arc(float x, float y, float r, float startDeg, float endDeg)
{
    HPDF_Page_Arc(m_page, x, y, r, startDeg, endDeg);
}

void
PdfPage::getPos(float& x, float& y)
{
    auto pos = HPDF_Page_GetCurrentPos(m_page);
    x = pos.x;
    y = pos.y;
}

void
PdfPage::stroke()
{
    HPDF_Page_Stroke(m_page);
}

void
PdfPage::save()
{
    HPDF_Page_GSave(m_page);
}

void
PdfPage::restore()
{
    HPDF_Page_GRestore(m_page);
}

void
PdfPage::translate(float x, float y)
{
    HPDF_Page_Concat(m_page, 1.0f, 0.0f, 0.0f, 1.0f, x, y);
}

void
PdfPage::curveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    HPDF_Page_CurveTo(m_page, x1, y1, x2, y2, x3, y3);
}


void
PdfPage::closePath()
{
    HPDF_Page_ClosePathStroke(m_page);
}

void
PdfPage::clip()
{
    //HPDF_Page_Eoclip(m_page);   // this clips to a path
    //HPDF_Page_Clip(m_page);
    HPDF_Page_Clip(m_page);
    HPDF_Page_EndPath(m_page);
}

} /* end namespace psc::pdf */