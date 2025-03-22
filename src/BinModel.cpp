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
#include <iomanip>


#include "BinModel.hpp"

BinModel::BinModel(Glib::Dispatcher& binDispatcher, Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
: m_binDispatcher{binDispatcher}
, m_pixbuf{pixbuf}
, m_max{0u}
{
    for (uint32_t rgb = 0; rgb < N_COL; ++rgb) {
        for (uint32_t c = 0; c < N_BIN; ++c) {
            m_bin[c][rgb] = 0u;
        }
        m_weight[rgb] = 0.0;
    }
}


void
BinModel::readPixbuf()
{
    m_max = 0u;     // avoid displaying intermediate values
    // assume colorspace RGB 8bit as the doc says these values are the only support
    if (m_pixbuf->get_colorspace() != Gdk::COLORSPACE_RGB) {
        std::cerr << "BinModel::readPixbuf expecting colorspace rgb is " << m_pixbuf->get_colorspace() << " continue (may display invalid results)." << std::endl;
    }
    if (m_pixbuf->get_bits_per_sample() != 8) {
        std::cerr << "BinModel::readPixbuf expecting 8bits per sample is " << m_pixbuf->get_bits_per_sample() << " continue (may display invalid results)." << std::endl;
    }
    uint32_t nChannels = m_pixbuf->get_n_channels();
    uint32_t width = m_pixbuf->get_width();
    for (int32_t y = 0; y < m_pixbuf->get_height(); ++y) {
        guint8* a = m_pixbuf->get_pixels() + y * m_pixbuf->get_rowstride();
        for (uint32_t x = 0; x < width; ++x) {
            for (uint32_t c = 0; c < nChannels; ++c) {
                if (c < N_COL) {
                    ++m_bin[*(a + c)][c];
                }
            }
            a += m_pixbuf->get_n_channels();
        }
    }
    m_pixbuf.reset();       // reset reference as we don't need it anymore (clear/release is a totally different story!)
    for (uint32_t c = 0; c < N_BIN; ++c) {
        for (uint32_t rgb = 0; rgb < N_COL; ++rgb) {
            if (m_bin[c][rgb] > m_max) {
                m_max = m_bin[c][rgb];
            }
            m_weight[rgb] += m_bin[c][rgb] * c;
        }
    }
    m_binDispatcher.emit();
}

uint32_t
BinModel::getMax()
{
    return m_max;
}

uint32_t
BinModel::getBin(uint32_t c, uint32_t rgb)
{
    return m_bin[c][rgb];
}


double
BinModel::getWeight(uint32_t rgb)
{
    return m_weight[rgb];
}
