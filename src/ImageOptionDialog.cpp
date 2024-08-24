/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include "ImageOptionDialog.hpp"
#include "ImageOptions.hpp"

ImageOptionDialog::ImageOptionDialog(Gtk::Window& parent, Gdk::PixbufFormat& format,
	Glib::RefPtr<Gio::File>& file, std::map<Glib::ustring, Glib::ustring> &opts)
: Gtk::Dialog("Options", parent, true)
, m_format{format}
{
    add_button("Cancel", Gtk::RESPONSE_CANCEL);
    Gtk::Button* okBtn = add_button("Ok", Gtk::RESPONSE_OK);

	m_opts = ImageOptions::getOptions(format, opts);
	if (m_opts) {
		int row = 0;
		for (auto opt : m_opts->getOptions()) {
			auto imgOpt = opt.second;
			auto grid =  Gtk::make_managed<Gtk::Grid>();
			grid->set_column_homogeneous(true);
			grid->set_column_spacing(5);
			get_content_area()->pack_start(*grid, true, true);
			auto adjustOpt = dynamic_cast<AdjustmentImageOption*>(imgOpt.get());
			auto textOpt = dynamic_cast<TextImageOption*>(imgOpt.get());
			Gtk::Widget* wdgt = nullptr;
			if (adjustOpt) {
				Gtk::Scale* scale = Gtk::make_managed<Gtk::Scale>(adjustOpt->getAdjustment());
				scale->set_digits(0);
				wdgt = scale;
			}
			else if (textOpt) {
				wdgt = Gtk::make_managed<Gtk::Entry>(textOpt->getBuffer());
			}
			else  {
				wdgt = Gtk::make_managed<Gtk::Label>("No option handling");
			}
			grid->attach(*wdgt, 1, row);
			Gtk::CheckButton* checkBtn = Gtk::make_managed<Gtk::CheckButton>(imgOpt->getName());
			checkBtn->set_active(imgOpt->isExport());
			if (wdgt) {
				wdgt->set_sensitive(imgOpt->isExport());
			}
			checkBtn->signal_toggled().connect([=] () {
				imgOpt->setExport(checkBtn->get_active());
				if (wdgt) {
					wdgt->set_sensitive(checkBtn->get_active());
				}
			});
			grid->attach(*checkBtn, 0, row);
			++row;
		}
	}
	else {
		std::cerr << "No options for format " << format.get_name() << " use hasOptions!" << std::endl;
	}
    if (file->query_exists()) {
        auto msg = Glib::ustring::sprintf("File %s exists. Overwrite?", file->get_path());
		Gtk::CheckButton* checkOverwriteBtn = Gtk::make_managed<Gtk::CheckButton>(msg);
		checkOverwriteBtn->signal_toggled().connect([=] () {
			okBtn->set_sensitive(checkOverwriteBtn->get_active());
		});
		get_content_area()->pack_start(*checkOverwriteBtn);
		okBtn->set_sensitive(false);
    }

	show_all();
}

bool
ImageOptionDialog::hasOptions(Gdk::PixbufFormat& format,
	Glib::RefPtr<Gio::File>& file, std::map<Glib::ustring, Glib::ustring>& existOpts)
{
	auto opts = ImageOptions::getOptions(format, existOpts);
	if (opts) {
		return true;
	}
	if (file->query_exists()) {
		return true;
	}
	if (!existOpts.empty()) {
		return true;
	}
	return false;
}


void
ImageOptionDialog::addOptions(std::vector<Glib::ustring>& keys, std::vector<Glib::ustring>& opts)
{
	if (m_opts) {
		for (auto opt : m_opts->getOptions()) {
			auto imgOpt = opt.second;
			Glib::ustring value = imgOpt->getValue();
			// max use gdk_pixbuf_format_is_save_option_supported
			if (imgOpt->isExport()
			 &&	!value.empty()) {	// empty -> not used...
				keys.push_back(imgOpt->getOutputKey());
				opts.push_back(value);
			}
		}
	}

	// These might also be helpful
	//ICO images can be saved in depth 16, 24, or 32, by using the "depth" parameter.
	// When the ICO saver is given "x_hot" and "y_hot" parameters, it produces a CUR instead of an ICO.
}



