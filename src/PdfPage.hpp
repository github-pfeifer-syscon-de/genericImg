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

namespace psc::pdf
{

class PdfExport;
class PdfFont;
class PdfImage;
class PdfFormat;
enum class Orientation;


class PdfPage
{
public:
    PdfPage(std::shared_ptr<PdfExport> pdfExport);
    explicit PdfPage(const PdfPage& orig) = delete;
    virtual ~PdfPage() = default;

    void setRgb(float r, float g, float b);
    void fill();
    void setLineWidth(float line_width);
    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void circle(float x, float y, float r);
    void arc(float x, float y, float r, float startDeg, float endDeg);
    void getPos(float& x, float& y);
    void stroke();
    void save();
    void restore();
    void translate(float x, float y);
    void curveTo(float x1, float y1, float x2, float y2, float x3, float y3);
    void closePath();
    void clip();
    void setFormat(PdfFormat fmt, Orientation orient);
    void getFormat(float& widthMM, float& heightMM);

    // using std::string by intention as in this stage
    //   this is considered binary data (but newlines are evaluated)
    //   see PdfExport createFont...
    void drawText(const std::string& text, float x, float y);
    // this does the conversion internally
    void drawText(const Glib::ustring& text, float x, float y);
    void drawImage(std::shared_ptr<PdfImage>& image, float x, float y, float w, float h);
    float getHeight();
    float getWidth();
    void setFont(std::shared_ptr<PdfFont>& font, float size);
    std::shared_ptr<PdfFont> getFont();
    float getFontSize();
    float getWordSpace();
    float getCharSpace();

protected:
    std::shared_ptr<PdfExport> m_pdfExport;
    HPDF_Destination m_dst{nullptr};
    HPDF_Page m_page{nullptr};
    std::shared_ptr<PdfFont> m_font;
    float m_fontSize{12.0};
};

} /* end namespace psc::pdf */
