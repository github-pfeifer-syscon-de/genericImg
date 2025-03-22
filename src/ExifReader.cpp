/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2021 rpf
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
#include <libexif/exif-data.h>

#include "ExifReader.hpp"
#include "ImageList.hpp"

ExifReader::ExifReader(ImageList& imageList)
: m_imageList{imageList}
, m_iterExif(imageList.appendList("Exif", ""))
{
}

void
ExifReader::append2IdfList(const char *name, Glib::ustring& value)
{
	m_imageList.appendList(m_chldsIdf, name, value);
}

void
ExifReader::append2ExifList(const char *ifdName)
{
	m_chldsIdf = m_imageList.appendList(m_iterExif, ifdName, "");
}

static void
exifEntryList(ExifEntry *exifEntry, void *user_data)
{
	ExifReader* exifReader = (ExifReader*)user_data;
	char val[1024];

	ExifIfd exifIfd = exif_entry_get_ifd(exifEntry);
	if (exifIfd != EXIF_IFD_COUNT) {
		exif_entry_get_value(exifEntry, val, sizeof(val));
		Glib::ustring value(val);
		const char* name = exif_tag_get_name_in_ifd(exifEntry->tag, exifIfd);
		if (!name) {
			name = "?";
		}
		//const char* ifdName = exif_ifd_get_name(exifIfd);
		//std::cout << "Ifd " << ifdName << " name " << name << " = " << val << std::endl;
		exifReader->append2IdfList(name, value);
	}
	else {
		std::cerr << "Coud not extract ifd tag "
			      << std::hex << exifEntry->tag << std::dec << std::endl;
	}
}


static void
exifContentList(ExifContent *exifCont, void *user_data)
{
	ExifReader* exifReader = (ExifReader*)user_data;
	ExifIfd exifIfd = exif_content_get_ifd(exifCont);
	if (exifIfd != EXIF_IFD_COUNT) {
		const char* ifdName = exif_ifd_get_name(exifIfd);
		if (!ifdName) {
			ifdName = "?";
		}

		exifReader->append2ExifList(ifdName);

		//std::cerr << "Reading ifd "  << ifdName << std::endl;
		//if (exifIfd != EXIF_IFD_1) {	// if this is not for thumbnail
		exif_content_foreach_entry(exifCont, exifEntryList, user_data);
		//}
		//else {
		//	std::cerr << "Skipped thumbnail " << ifdName << std::endl;
		//}
	}
	else {
		std::cerr << "Coud not extract ifd content "  << exifCont->count << std::endl;
	}
}

void
ExifReader::decode(const Glib::ustring& exifStr)
{
	uint32_t len;
	uint8_t* buf = toBuffer(exifStr, len);
	if (buf) {
		ExifData* exifData = exif_data_new_from_data(buf, len);
		if (exifData) {
			exif_data_foreach_content(exifData, exifContentList, this);
			//exif_data_dump(exifData);
			exif_data_free(exifData);
		}
		delete[] buf;
	}
}

static inline uint32_t
hexValue(uint8_t chr)
{
	if (chr >= 'a') {
		return 10 + chr - 'a';
	}
	if (chr >= 'A') {
		return 10 + chr - 'A';
	}
	return chr - '0';
}

uint8_t*
ExifReader::toBuffer(const Glib::ustring& exifStr, uint32_t &retLen)
{
	if (exifStr.length() < 20
	 ||	exifStr.substr(0, 5) != "\nexif") {
		std::cerr << "No starting exif \"" << exifStr.substr(0, std::min(5, (int)exifStr.length())) << "\"" << std::endl;
		return nullptr;
	}
	size_t pos = exifStr.find('\n', 7);
	if (pos == exifStr.npos) {
		std::cerr << "No delimiting newline \"" << exifStr.substr(7, std::min(20, (int)exifStr.length() - 7)) << "\"" << std::endl;
		return nullptr;
	}
	int32_t len = atoi(exifStr.substr(6, pos - 6).c_str());
	if (len < 0 || len > 65536) {
		std::cerr << "Unexpected length \"" << exifStr.substr(7, std::min(20, (int)exifStr.length() - 7)) << "\" decoded " << len << std::endl;
		return nullptr;
	}
	//std::cout << "Decoded len " << len << std::endl;
	uint8_t* buf = new uint8_t[len];
	if (!buf) {
		std::cerr << "Coud not allocate length " << len << std::endl;
		return nullptr;
	}
	retLen = len;
	++pos;		// hexdump starts after newline
	const char* exifChrs = exifStr.c_str();	// expecting only ascii (the utf8 handling will be time consuming, and unnecessary)
	exifChrs += pos;
	for (int32_t i = 0; i < len; ++i) {
		while (*exifChrs != '\0'
			&& *exifChrs < '0') {	// somewhere line breaks will appear -> skip any control
			++exifChrs;
		}
		uint8_t chr = *exifChrs;
		if (chr == '\0')
			break;
		uint32_t iHex = hexValue(chr) << 4;
		++exifChrs;
		chr = *exifChrs;
		if (chr == '\0')
			break;
		iHex |= hexValue(chr);
		++exifChrs;
		buf[i] = (uint8_t)iHex;
	}
	return buf;
}