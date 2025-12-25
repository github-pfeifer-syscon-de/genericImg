/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf 
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

#include <StringUtils.hpp>
#include <algorithm>

#include "ImageFileChooser.hpp"

ImageFileChooser::ImageFileChooser(
        Gtk::Window& win,
        bool save,
        const std::vector<std::string>& types)
: ImageFileChooser(win, save, createFilter())
{
    auto filter = get_filter();
    filter->add_custom(Gtk::FileFilterFlags::FILE_FILTER_FILENAME, sigc::mem_fun(*this, &ImageFileChooser::acceptFile));
    Glib::ustring allTypes;
    for (auto& type : types) {
        m_types.insert(StringUtils::lower(type));
        if (!allTypes.empty()) {
            allTypes += ", ";
        }
        allTypes += type;
    }
    set_title(save
              ? Glib::ustring::sprintf("Save %s-file(s)", allTypes)
              : Glib::ustring::sprintf("Open %s-file(s)", allTypes));

}

Glib::RefPtr<Gtk::FileFilter>
ImageFileChooser::createFilter()
{
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Images");
    //filter->add_pixbuf_formats();
    return filter;
}

ImageFileChooser::ImageFileChooser(
        Gtk::Window& win,
        bool save,
        const Glib::RefPtr<Gtk::FileFilter>& filter)
: Gtk::FileChooserDialog(win
                        , ""
                        , save
                        ? Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE
                        : Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN
                        , Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT)

{
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    add_button(save
                ? "_Save"
                : "_Open", Gtk::RESPONSE_ACCEPT);
    set_title(save
              ? "Save image-file"
              : "Open image-file");
    set_filter(filter);
}

bool
ImageFileChooser::acceptFile(const Gtk::FileFilter::Info& filter_info)
{
    auto ext = StringUtils::getExtension(filter_info.filename);
    return m_types.contains(StringUtils::lower(ext));
}

