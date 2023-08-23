/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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

#pragma once

#include <vector>
#include <memory>
#include <gtkmm.h>

//template<class T,T::BaseObjectType>
//class ImageView;
class DisplayImage;

class ViewIntf
{
public:
    virtual void setFile(const Glib::RefPtr<Gio::File>& file) = 0;
    virtual void on_menu_n(gint n) = 0;
    virtual void setDisplayImage(Glib::RefPtr<DisplayImage>& pixbuf) = 0;

};

typedef void (ViewIntf::*function_ptr)(int);
class Mode
{
public:
    Mode() = default;
    virtual ~Mode() = default;

    virtual void show(ViewIntf* viewIntf) = 0;
    virtual void buildMenu(Gtk::Menu* subMenu, ViewIntf* imageView, function_ptr fun) = 0;
    virtual bool isComplete() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;
    virtual bool hasNavigation() = 0;
    virtual void set(int32_t n) = 0;
    virtual bool join(std::shared_ptr<Mode> other) = 0;
};

class PagingMode
: public Mode
{
public:
    PagingMode(int32_t front, std::vector<Glib::RefPtr<Gio::File>>& picts);
    explicit PagingMode(const PagingMode& other) = delete;
    virtual ~PagingMode() = default;

    bool isComplete() override;
    void show(ViewIntf* viewIntf) override;
    void buildMenu(Gtk::Menu* subMenu, ViewIntf* imageView, function_ptr fun) override;
    void prev() override;
    void next() override;
    void set(int32_t n) override;
    bool join(std::shared_ptr<Mode> other) override;
    bool hasNavigation() override;

    int32_t get();
    std::vector<Glib::RefPtr<Gio::File>> getPicts();
protected:
    Glib::RefPtr<Gio::File> getFrontFile();

private:
    int32_t m_front;
    std::vector<Glib::RefPtr<Gio::File>> m_picts;
};



