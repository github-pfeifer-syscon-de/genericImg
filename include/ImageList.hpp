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

#pragma once

#include <gtkmm.h>


/*
 * model of image property display
 */
class VariableColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> m_name;
    Gtk::TreeModelColumn<Glib::ustring> m_value;
    VariableColumns()
    {
        add(m_name);
        add(m_value);
    }
};

class DisplayImage;

class ImageList
: public Gtk::TreeStore
{
public:
    virtual ~ImageList() = default;
    static Glib::RefPtr<ImageList> create();

    void fillList(const Glib::RefPtr<Gio::File> file);
    static VariableColumns m_variableColumns;
    void fillList(Glib::RefPtr<DisplayImage>& pixbuf);

    Gtk::TreeIter appendList(const char *name, const Glib::ustring& value);
    Gtk::TreeIter appendList(Gtk::TreeIter& node, const char *name, const Glib::ustring& value);
protected:
    void appendList(Gtk::TreeIter& node, const char *name, const Glib::DateTime& modified);
    static Glib::ustring formatScale(guint64 value);    // , const char *suffix, double scale = 1024.0
    void setValues(Gtk::TreeIter& i, const char *name, const Glib::ustring& value);
    std::map<Glib::ustring, Glib::ustring> getOptions(Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
private:
    ImageList();
};

