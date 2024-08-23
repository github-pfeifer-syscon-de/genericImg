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
#include <future>
#include <memory>

#include "BinModel.hpp"

class DisplayImage;

class BinView
: public Gtk::DrawingArea
{
public:
    BinView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~BinView() = default;

    void setPixbuf(Glib::RefPtr<DisplayImage>& pixbuf);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    double getColorR(uint32_t rgb);
    double getColorG(uint32_t rgb);
    double getColorB(uint32_t rgb);
private:
    Glib::Dispatcher m_binDispatcher;
    std::future<void> m_pixelReader;
    std::shared_ptr<BinModel> m_model;

    static const int32_t m_base = 16;
    static const int32_t m_weightWidth = 96;

    void drawBottomGradient(const Cairo::RefPtr<Cairo::Context>& cr, Gtk::Allocation& allocation);
    void drawWeightGradient(const Cairo::RefPtr<Cairo::Context>& cr, Gtk::Allocation& allocation);
};
