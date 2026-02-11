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
#include <string_view>
#include <hpdf.h>
#include <StringUtils.hpp>
#include <fontconfig/fontconfig.h>

#include "TableProperties.hpp"
#include "PdfExport.hpp"
#include "PdfFont.hpp"
#include "PdfPage.hpp"
#include "genericimg_config.h"

namespace psc::pdf
{

static void
#ifdef HPDF_DLL
__stdcall
#endif
error_handler(HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    //printf ("PdfOut ERROR: error_no=0x%04X, detail_no=%u\n",
    //        (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
    auto msg = psc::fmt::format("HPDF_Error error {:#04x} detail {}"
                                        , error_no, detail_no);
    throw PdfException(msg);
}

PdfExport::PdfExport()
{
    m_pdf = HPDF_New(error_handler, nullptr);   //
    if (!m_pdf) {
        printf("error: cannot create PdfDoc object\n");
        throw PdfException("PdfExport::PdfExport");
    }

    HPDF_SetCompressionMode(m_pdf, HPDF_COMP_ALL);
}

PdfExport::~PdfExport()
{
    if (m_pdf) {
        HPDF_Free(m_pdf);
        m_pdf = nullptr;
    }
}


PdfFormat
PdfExport::getFormat()
{
    return m_format;
}

void
PdfExport::setFormat(PdfFormat pdfFormat)
{
    m_format = pdfFormat;
}

Orientation
PdfExport::getOrientation()
{
    return m_orientation;
}

void
PdfExport::setOrientation(Orientation orient)
{
    m_orientation = orient;
}


HPDF_Doc
PdfExport::getDoc()
{
    return m_pdf;
}

void
PdfExport::save(const std::string& filename)
{
    if (m_lastPage) {
        m_lastPage->endText();
    }
    /* save the document to a file */
    HPDF_SaveToFile(m_pdf, filename.c_str());
}

std::shared_ptr<PdfFont>
PdfExport::createFont(std::string_view fontName)
{
    //std::cout << "PdfFont::PdfFont name " << font_name << std::endl;
    return std::make_shared<PdfFont>(m_pdf, fontName.data(), "");
}

std::shared_ptr<PdfFont>
PdfExport::createFontInternalWithEncoding(std::string_view encoding)
{
    auto afmFile = findFontFile("ua1r8a.afm");
    auto pfbFile = findFontFile("ua1r8a.pfb");
    if (!afmFile || !pfbFile) {
        std::cout << "Error accessing font files from package data/resources" << std::endl;
    }
    auto afmName = afmFile->get_path();
    auto pfpName = pfbFile->get_path();
    //std::cout << "PdfExport::createFontInternalWithEncoding " << afmName << " exist " << std::boolalpha << afmFile->query_exists() << std::endl;
    return createFontType1(afmName, pfpName, encoding);
}

std::shared_ptr<PdfFont>
PdfExport::createFontType1(const std::string& afmName, const std::string& pfpName, std::string_view encoding)
{
    //std::cout << "PdfFont::PdfFont afm " << afmName << " exists " << std::boolalpha << afmFile->query_exists()
    //          << " pfp " << pfpName << " exists " << std::boolalpha << pfbFile->query_exists() << std::endl;
    auto fontName = HPDF_LoadType1FontFromFile(m_pdf, afmName.c_str(), pfpName.c_str());
    return std::make_shared<PdfFont>(m_pdf, fontName, encoding);
}

std::shared_ptr<PdfFont>
PdfExport::createFontTTFMatch(const std::string& name, std::string_view encoding, bool embedd)
{
    std::shared_ptr<PdfFont> pdfFont;
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pat = FcNameParse((const FcChar8*) (name.c_str()));
    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    // see https://cgit.freedesktop.org/fontconfig/tree/fc-match/fc-match.c
    FcResult result;
	FcFontSet* font_patterns = FcFontSort(0, pat, FcTrue, 0, &result);  // FcFalse list all
	if (font_patterns) {
        for (int32_t j = 0; j < font_patterns->nfont; j++) {
            auto font = font_patterns->fonts[j];
            FcChar8* file = nullptr;
            if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
                std::string fontFile = ((char*) file);
                if (!pdfFont && StringUtils::endsWith(fontFile, ".ttf")) {  // with .ttc collections which index to choose?
                    pdfFont = createFontTTF(fontFile, encoding, 0, embedd);
                }
            }
            FcPatternDestroy(font);
        }
    }
    else  {
	    std::cout << "No font matching " << name << " found!" << std::endl;
    }
	FcFontSetSortDestroy(font_patterns);
    FcPatternDestroy(pat);
    return pdfFont;
}

std::shared_ptr<PdfFont>
PdfExport::createFontTTF(const std::string& ttfName, std::string_view encoding, int idx, bool embedd)
{
    if (encoding.substr(0, 3) == "UTF") {       // maybe more possible encodings?
        HPDF_UseUTFEncodings(m_pdf);
        HPDF_SetCurrentEncoder(m_pdf, encoding.data());
    }
    const char* detail_font_name;
    auto pdfEmbedd = embedd ? HPDF_TRUE : HPDF_FALSE;
    if (StringUtils::endsWith(ttfName, ".ttc")) {
        detail_font_name = HPDF_LoadTTFontFromFile2(m_pdf, ttfName.data(), idx, pdfEmbedd);
    }
    else {
        detail_font_name = HPDF_LoadTTFontFromFile(m_pdf, ttfName.data(), pdfEmbedd);
    }
    return std::make_shared<PdfFont>(m_pdf, detail_font_name, encoding);
}


Glib::RefPtr<Gio::File>
PdfExport::findFontFile(const char* file)
{
    // if possible access from package dir, as this is simpler
    auto fullPath = Glib::canonicalize_filename(file, PACKAGE_DATA_DIR);
    auto pkgFile = Gio::File::create_for_path(fullPath);
    //std::cout << "PdfExport::findFontFile check global " << fullPath << " exist " << std::boolalpha << file->query_exists() << std::endl;
    if (pkgFile->query_exists()) {
        return pkgFile;
    }
    // fallback to resources
    auto resName = std::string(psc::ui::TableProperties::RESOURCE_PREFIX) + "/" + file;
    //std::cout << "PdfFont::findFontFile res " << resName << std::endl;
    Glib::RefPtr<const Glib::Bytes> data = Gio::Resource::lookup_data_global(resName);
    //std::cout << "PdfFont::findFontFile data " << data->get_size() << std::endl;
    Glib::RefPtr<Gio::File> tempDir = Gio::File::create_for_path(Glib::get_tmp_dir());
    Glib::RefPtr<Gio::File> dataFile = tempDir->get_child(file);
    if (!dataFile->query_exists()) {
        try {
            auto strm = dataFile->create_file(Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
            strm->write_bytes(data);
            //std::cout << "PdfFont::findFontFile written " << len << std::endl;
            strm->close();
        }
        catch (const Glib::Error& err) {
            std::cout << "PdfFont::findFontFile error writing " << dataFile->get_path() << std::endl;
            dataFile->remove();
            return Glib::RefPtr<Gio::File>();
        }
    }
    return dataFile;
}

std::shared_ptr<PdfPage>
PdfExport::createPage()
{
    if (m_lastPage) {
        m_lastPage->endText();
    }
    /* add a new page object. */
    auto page = HPDF_AddPage(m_pdf);
    ++m_pageNum;
    m_lastPage = std::make_shared<PdfPage>(page, this);
    return m_lastPage;
}

void
PdfExport::setPageNumStyle(uint32_t startPage
                        , uint32_t pageNum
                        , std::string_view prefix
                        , bool roman
                        , bool letter
                        , bool upper)
{
    HPDF_PageNumStyle style = HPDF_PAGE_NUM_STYLE_DECIMAL;
    if (roman) {
        style = upper
                ? HPDF_PAGE_NUM_STYLE_UPPER_ROMAN
                : HPDF_PAGE_NUM_STYLE_LOWER_ROMAN;
    }
    else if (letter) {
        style = upper
                ? HPDF_PAGE_NUM_STYLE_UPPER_LETTERS
                : HPDF_PAGE_NUM_STYLE_LOWER_LETTERS;

    }
    HPDF_AddPageLabel(m_pdf
                     , startPage    /* The first page that applies this labeling range.  */
                     ,  style       /* style, */
                     , pageNum      /* first_page, */
                     , prefix == "" ? nullptr : prefix.data());    /* *prefix */
}

uint32_t
PdfExport::getPageNum()
{
    return m_pageNum;
}

float
PdfExport::mm2dot(float mm)
{
    //  72 dot per inch
    return mm  * 72.f / 25.4f;
}

float
PdfExport::dot2mm(float dot)
{
    //  25.4mm per inch
    return dot * 25.4f / 72.f;
}

} /* end namespace psc::pdf */
