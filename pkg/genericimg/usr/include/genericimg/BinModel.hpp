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

class BinModel  {
public:
    BinModel(Glib::Dispatcher& binDispatcher, Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
    virtual ~BinModel() = default;

    void readPixbuf();
    uint32_t getMax();
    uint32_t getBin(uint32_t c, uint32_t rgb);
    double getWeight(uint32_t rgb);
    static const uint32_t N_BIN = 256;
    static const uint32_t N_COL = 3;
protected:

private:
    Glib::Dispatcher& m_binDispatcher;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    uint32_t m_bin[N_BIN][N_COL];
    uint32_t m_max;
    double m_weight[N_COL];
};
