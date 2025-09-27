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
#include <cairomm/cairomm.h>

#include "PdfImage.hpp"
#include "PdfExport.hpp"

namespace psc::pdf
{

PdfImage::PdfImage(std::shared_ptr<PdfExport>& pdfExport)
: m_pdfExport{pdfExport}
{
}

float
PdfImage::getWidth()
{
    return static_cast<float>(HPDF_Image_GetWidth(m_image));
}

float
PdfImage::getHeight()
{
    return static_cast<float>(HPDF_Image_GetHeight(m_image));
}

void
PdfImage::loadPng(const std::string& filename)
{
    HPDF_Doc pdf = m_pdfExport->getDoc();
    m_image = HPDF_LoadPngImageFromFile(pdf, filename.c_str());
}

void
PdfImage::load(const Cairo::RefPtr<Cairo::ImageSurface>& cimage)
{
    if (cimage->get_format() != Cairo::Format::FORMAT_RGB24) {
        std::cout << "PdfImage::load unable to handle format " << cimage->get_format() << std::endl;
    }
    // argb to packed rgb
    HPDF_UINT width = cimage->get_width();
    HPDF_UINT height = cimage->get_height();
    auto rgbPackedData = std::vector<uint8_t>(width * height * 3);
    auto rgbPackedPtr = &rgbPackedData[0];
    auto ptrCairoImg = reinterpret_cast<uint32_t*>(cimage->get_data());
    for (uint32_t r = 0; r < height; ++r) {
        for (uint32_t w = 0; w < width; ++w) {
            auto argb = ptrCairoImg[w];
            // tested with little-endian (intel) (unsure if this is correct for big endian?)
            *rgbPackedPtr++ = ((argb >> 16) & 0xff);       // R
            *rgbPackedPtr++ = ((argb >> 8) & 0xff);        // G
            *rgbPackedPtr++ = (argb & 0xff);               // B
        }
        ptrCairoImg += width;
    }
    //std::cout << "cimage " << width << " height " << height << std::endl;
    HPDF_Doc pdf = m_pdfExport->getDoc();
    m_image = HPDF_LoadRawImageFromMem(pdf,
      rgbPackedData.data(), width, height, HPDF_CS_DEVICE_RGB, 8);
}


HPDF_Image
PdfImage::getPdfImage()
{
    return m_image;
}

} /* end namespace psc::pdf */
