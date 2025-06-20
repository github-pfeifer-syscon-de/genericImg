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
#include <memory>
#include <hpdf.h>

namespace psc::pdf
{

class PdfExport;

class PdfImage {
public:
    PdfImage(std::shared_ptr<PdfExport>& pdfExport);
    explicit PdfImage(const PdfImage& orig) = delete;
    virtual ~PdfImage() = default;

    float getWidth();
    float getHeight();
    HPDF_Image getPdfImage();
    void loadPng(const Glib::ustring& filename);
    void load(const Cairo::RefPtr<Cairo::ImageSurface>& cimage);

private:
    HPDF_Image m_image{nullptr};
    std::shared_ptr<PdfExport> m_pdfExport;
};

} /* end namespace psc::pdf */
