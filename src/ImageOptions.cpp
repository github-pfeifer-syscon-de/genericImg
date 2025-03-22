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

#include "ImageOptions.hpp"


ImageOption::ImageOption(bool exp)
: m_export{exp}
{
}

void
ImageOption::setExport(bool exp)
{
	m_export = exp;
}

bool
ImageOption::isExport()
{
	return m_export;
}

ImageOptions::ImageOptions()
{
}

ImageOptions::ImageOptions(const ImageOptions& other)
: m_options(other.m_options)
{
}

ImageOptions::~ImageOptions()
{
}

const std::map<Glib::ustring, std::shared_ptr<ImageOption>>&
ImageOptions::getOptions()
{
	return m_options;
}

void
ImageOptions::add(std::map<Glib::ustring, Glib::ustring>& opts)
{
	for (auto m : opts) {
		auto i = m_options.find(m.first);
		if (i != m_options.end()) {
			auto p = (*i).second;
			p->setValue(m.second);
		}
		else {
			Glib::ustring key(m.first);
			if (key.length() >= 6
			 && key.substr(0, 6) == "tEXt::") {		// dont display tEXt prefix
				key = key.substr(6);
			}
			auto opt = std::make_shared<GenericImageOption>(key, m.first, false);
			opt->setValue(m.second);
			m_options.insert(std::make_pair(m.first, opt));
		}
	}
}

std::shared_ptr<ImageOptions>
ImageOptions::getOptions(Gdk::PixbufFormat& format, std::map<Glib::ustring, Glib::ustring>& opts)
{
	auto jpgOptions = std::make_shared<JpegImageOptions>();
	if (jpgOptions->matches(format)) {
		jpgOptions->add(opts);
		return jpgOptions;
	}
	auto pngOptions = std::make_shared<PngImageOptions>();
	if (pngOptions->matches(format)) {
		pngOptions->add(opts);
		return pngOptions;
	}
	std::shared_ptr<ImageOptions> options;
	return options;
}



AdjustmentImageOption::AdjustmentImageOption(bool exp)
: ImageOption(exp)
, m_adjustment(Gtk::Adjustment::create(0, 0, 0))
{
}

AdjustmentImageOption::~AdjustmentImageOption()
{
}

Glib::RefPtr<Gtk::Adjustment>
AdjustmentImageOption::getAdjustment()
{
	return m_adjustment;
}

Glib::ustring
AdjustmentImageOption::getValue()
{
	return Glib::ustring::sprintf("%.0f", m_adjustment->get_value());
}

void
AdjustmentImageOption::setValue(Glib::ustring& value)
{
	int val;
	if (sscanf(value.c_str(), "%d", &val) == 1) {
		m_adjustment->set_value(val);
	}
}


QualityImageOption::QualityImageOption()
: AdjustmentImageOption(true)
{
	m_adjustment->set_lower(getMin());
	m_adjustment->set_upper(getMax());
	m_adjustment->set_value(getDefault());
}


QualityImageOption::~QualityImageOption()
{
}

int
QualityImageOption::getMin()
{
	return 0;
}

int
QualityImageOption::getMax()
{
	return 100;
}

int
QualityImageOption::getDefault()
{
	return 90;
}

const Glib::ustring
QualityImageOption::getName()
{
	return Glib::ustring("Quality");
}

const Glib::ustring
QualityImageOption::getOutputKey()
{
	return Glib::ustring("quality");
}

JpegImageOptions::JpegImageOptions()
{
	auto jpgOpt = std::make_shared<QualityImageOption>();
	m_options.insert(std::make_pair(jpgOpt->getOutputKey(), jpgOpt));
}

JpegImageOptions::~JpegImageOptions()
{
}

bool
JpegImageOptions::matches(Gdk::PixbufFormat& format)
{
	bool ret = format.get_name() == "jpeg";
	return ret;
}



TextImageOption::TextImageOption(bool exp)
: ImageOption(exp)
, m_buffer(Gtk::EntryBuffer::create())
{
}

TextImageOption::~TextImageOption()
{
}

Glib::RefPtr<Gtk::EntryBuffer>
TextImageOption::getBuffer()
{
	return m_buffer;
}

Glib::ustring
TextImageOption::getValue()
{
	return m_buffer->get_text();
}

void
TextImageOption::setValue(Glib::ustring& value)
{
	m_buffer->set_text(value);
}


GenericImageOption::GenericImageOption(const Glib::ustring& key, const Glib::ustring& outputKey, bool exp)
: TextImageOption(exp)
, m_key(key)
, m_outputKey(outputKey)
{
	m_buffer->set_max_length(getMax());
}

GenericImageOption::~GenericImageOption()
{
}

int
GenericImageOption::getMax()
{
	return 79;
}

const Glib::ustring
GenericImageOption::getName()
{
	return m_key;
}

const Glib::ustring
GenericImageOption::getOutputKey()
{
	return m_outputKey;
}


PngImageOptions::PngImageOptions()
{
	auto titleOpt = std::make_shared<GenericImageOption>("Title", "tEXt::Title", true);      //  Short (one line) title or caption for image
	auto authorOpt = std::make_shared<GenericImageOption>("Author", "tEXt::Author", true);    // Name of image's creator
	auto descOpt = std::make_shared<GenericImageOption>("Description", "tEXt::Description", true); // Description of image (possibly long)
	auto copyOpt = std::make_shared<GenericImageOption>("Copyright", "tEXt::Copyright", true);   // Copyright notice
	m_options.insert(std::make_pair(titleOpt->getOutputKey(), titleOpt));
	m_options.insert(std::make_pair(authorOpt->getOutputKey(), authorOpt));
	m_options.insert(std::make_pair(descOpt->getOutputKey(), descOpt));
	m_options.insert(std::make_pair(copyOpt->getOutputKey(), copyOpt));
	// more options (not used)
	//    Creation Time    Time of original image creation
	//    Software         Software used to create the image
	//    Disclaimer       Legal disclaimer
	//    Warning          Warning of nature of content
	//    Source           Device used to create the image
	//    Comment          Miscellaneous comment; conversion from GIF comment
}

PngImageOptions::~PngImageOptions()
{
}

bool
PngImageOptions::matches(Gdk::PixbufFormat& format)
{
	bool ret = format.get_name() == "png";
	return ret;
}

