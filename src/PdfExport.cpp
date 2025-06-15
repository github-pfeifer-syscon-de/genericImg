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
#include <setjmp.h>
#include <hpdf.h>
#include <psc_format.hpp>

#include "TableProperties.hpp"
#include "PdfExport.hpp"
#include "PdfFont.hpp"
#include "config.h"

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
    auto err = psc::fmt::format("HPDF_Error error {:#04x} detail {}"
                                , (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
    throw PdfException(err);
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
    /* save the document to a file */
    HPDF_SaveToFile(m_pdf, filename.c_str());
}

std::shared_ptr<PdfFont>
PdfExport::createFont(const Glib::ustring& fontName)
{
    //std::cout << "PdfFont::PdfFont name " << font_name << std::endl;
    auto font = HPDF_GetFont(m_pdf, fontName.c_str(), nullptr);
    return std::make_shared<PdfFont>(font);
}

std::shared_ptr<PdfFont>
PdfExport::createFontInternalWithEncoding(const Glib::ustring& encoding)
{
    auto afmFile = findFontFile("a010013l.afm");
    auto pfbFile = findFontFile("a010013l.pfb");
    if (!afmFile || !pfbFile) {
        std::cout << "Error accessing font files from package data/resources" << std::endl;
    }
    auto afmName = afmFile->get_path();
    auto pfpName = pfbFile->get_path();
    //std::cout << "PdfExport::createFontInternalWithEncoding " << afmName << " exist " << std::boolalpha << afmFile->query_exists() << std::endl;
    return createFontType1(afmName, pfpName, encoding);
}

std::shared_ptr<PdfFont>
PdfExport::createFontType1(const std::string& afmName, const std::string& pfpName, const Glib::ustring& encoding)
{
    //std::cout << "PdfFont::PdfFont afm " << afmName << " exists " << std::boolalpha << afmFile->query_exists()
    //          << " pfp " << pfpName << " exists " << std::boolalpha << pfbFile->query_exists() << std::endl;
    auto fontName = HPDF_LoadType1FontFromFile(m_pdf, afmName.c_str(), pfpName.c_str());
    auto font = HPDF_GetFont(m_pdf, fontName, encoding.c_str());
    return std::make_shared<PdfFont>(font);
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
            auto len = strm->write_bytes(data);
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

float
PdfExport::mm2dot(float mm)
{
    //  72 dot per inch
    return mm / 25.4f * 72.f;
}

