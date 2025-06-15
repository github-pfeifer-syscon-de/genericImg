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
#include <glibmm.h>

#include "PdfFont.hpp"
#include "PdfExport.hpp"
#include "TableProperties.hpp"

PdfFont::PdfFont(PdfExport* pdfExport, const Glib::ustring& name)
{
    HPDF_Doc pdf = pdfExport->getDoc();
    std::string font_name;
    const char* encoding = nullptr;
    auto expEncoding = pdfExport->getEncoding();
    std::cout << "PdfFont::PdfFont enc " << expEncoding << std::endl;
    if (!expEncoding.empty()) {
        auto afmFile = createTemp("a010013l.afm");
        auto pfbFile = createTemp("a010013l.pfb");
        if (!afmFile || !pfbFile) {
            std::cout << "Error creating font files from resources" << std::endl;
        }
        encoding = expEncoding.c_str();
        auto afmName = afmFile->get_path();
        auto pfpName = pfbFile->get_path();
        //std::cout << "PdfFont::PdfFont afm " << afmName << " exists " << std::boolalpha << afmFile->query_exists()
        //          << " pfp " << pfpName << " exists " << std::boolalpha << pfbFile->query_exists() << std::endl;
        font_name = HPDF_LoadType1FontFromFile(pdf, afmName.c_str(), pfpName.c_str());
    }
    else {
        font_name = name;
    }
    /* create default-font */
    //std::cout << "PdfFont::PdfFont name " << font_name << std::endl;
    m_font = HPDF_GetFont(pdf, font_name.c_str(), encoding);
}

Glib::RefPtr<Gio::File>
PdfFont::createTemp(const char* file)
{
    auto resName = std::string(psc::ui::TableProperties::RESOURCE_PREFIX) + "/" + file;
    //std::cout << "PdfFont::createTemp res " << resName << std::endl;
    Glib::RefPtr<const Glib::Bytes> data = Gio::Resource::lookup_data_global(resName);
    //std::cout << "PdfFont::createTemp data " << data->get_size() << std::endl;
    Glib::RefPtr<Gio::File> tempDir = Gio::File::create_for_path(Glib::get_tmp_dir());
    Glib::RefPtr<Gio::File> dataFile = tempDir->get_child(file);
    if (!dataFile->query_exists()) {
        try {
            auto strm = dataFile->create_file(Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
            auto len = strm->write_bytes(data);
            //std::cout << "PdfFont::createTemp written " << len << std::endl;
            strm->close();
        }
        catch (const Glib::Error& err) {
            std::cout << "Error writing " << dataFile->get_path() << std::endl;
            dataFile->remove();
            return Glib::RefPtr<Gio::File>();
        }
    }
    return dataFile;
}

HPDF_Font
PdfFont::getPdfFont()
{
    return m_font;
}
