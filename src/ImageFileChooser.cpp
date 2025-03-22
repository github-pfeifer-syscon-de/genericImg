/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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


#include "ImageFileChooser.hpp"

ImageFileChooser::ImageFileChooser(
        Gtk::Window& win,
        bool save,
        const std::vector<Glib::ustring>& types)
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
    Glib::ustring allTypes;
    for (auto type : types) {
        if (!allTypes.empty()) {
            allTypes += ", ";
        }
        allTypes += type;
    }
    set_title(save
              ? Glib::ustring::sprintf("Save %s-file(s)", allTypes)
              : Glib::ustring::sprintf("Open %s-file(s)", allTypes));

    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Type");
    for (auto type : types) {
        filter->add_pattern(Glib::ustring::sprintf("*.%s", type));
    }
    set_filter(filter);
}
