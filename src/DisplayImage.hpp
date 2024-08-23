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

#pragma once

#include <gtkmm.h>

class DisplayImage : public Glib::Object {
public:
    virtual ~DisplayImage() = default;
    static Glib::RefPtr<DisplayImage> create(Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

    Glib::RefPtr<DisplayImage> crop(int x0, int y0, int width, int height);
    Glib::RefPtr<Gdk::Pixbuf> getPixbuf();
    std::map<Glib::ustring, Glib::ustring> getOptions();

    int get_width();
    int get_height();

private:
    DisplayImage(Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
};
