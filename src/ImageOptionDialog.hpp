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



#pragma once

#include <gtkmm.h>
#include <vector>
#include <map>

class ImageOptions;

class ImageOptionDialog : public Gtk::Dialog {
public:
    ImageOptionDialog(Gtk::Window& parent, Gdk::PixbufFormat& format,
            Glib::RefPtr<Gio::File>& file, std::map<Glib::ustring, Glib::ustring>& opts);
    virtual ~ImageOptionDialog() = default;

    static bool hasOptions(Gdk::PixbufFormat& format,
            Glib::RefPtr<Gio::File>& file, std::map<Glib::ustring, Glib::ustring>& opts);
    void addOptions(std::vector<Glib::ustring>& keys, std::vector<Glib::ustring>& opts);
protected:
private:
    Gdk::PixbufFormat m_format;
    std::shared_ptr<ImageOptions> m_opts;
};

