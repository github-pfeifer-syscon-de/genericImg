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


#include "BinView.hpp"
#include "DisplayImage.hpp"

BinView::BinView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::DrawingArea(cobject)
, m_binDispatcher()
, m_model()
{
   m_binDispatcher.connect(sigc::mem_fun(*this, &BinView::queue_draw));

}

void
BinView::setPixbuf(Glib::RefPtr<DisplayImage>& displayImage)
{
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = displayImage->getPixbuf();
    m_model = std::make_shared<BinModel>(m_binDispatcher, pixbuf);
    m_pixelReader = std::async(std::launch::async, &BinModel::readPixbuf, m_model);
}


bool
BinView::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::DrawingArea::on_draw(cr);
    if (m_model) {
        uint32_t max = m_model->getMax();
        if (max > 0u) {
            Gtk::Allocation allocation = get_allocation();
            const int alloc_width = allocation.get_width();
            const int alloc_height = allocation.get_height() - m_base;
            drawWeightGradient(cr, allocation);
            cr->set_line_width(1.0);
            double fHorz = (double)alloc_width / (double)BinModel::N_BIN;
            for (uint32_t rgb = 0; rgb < BinModel::N_COL; ++rgb) {
                cr->set_source_rgb(getColorR(rgb),  getColorG(rgb),  getColorB(rgb));
                for (uint32_t c = 0; c < BinModel::N_BIN; ++c) {
                    uint32_t bin = m_model->getBin(c, rgb);
                    double v = (double)bin / (double)max;
                    double x = c * fHorz;
                    double y = (1.0 - v) * (double)alloc_height;
                    if (c == 0) {
                        cr->move_to(x, y);
                    }
                    else {
                        cr->line_to(x, y);
                    }
                }
                cr->stroke();
            }
            // draw a gradient to identify dark and light
            drawBottomGradient(cr, allocation);
        }
    }
    return true;
}

void
BinView::drawWeightGradient(const Cairo::RefPtr<Cairo::Context>& cr, Gtk::Allocation& allocation)
{
    const int alloc_width = allocation.get_width();
    double weight[3];
    double sum = 0.0;
    for (uint32_t rgb = 0; rgb < BinModel::N_COL; ++rgb) {
        weight[rgb] = m_model->getWeight(rgb);
        sum += weight[rgb];
    }
    for (uint32_t rgb = 0; rgb < BinModel::N_COL; ++rgb) {
        double x = alloc_width - m_weightWidth - 4;
        double y = (m_base + 4) * (rgb+1);
        Cairo::RefPtr<Cairo::LinearGradient> pat = Cairo::LinearGradient::create(x, y, x+m_weightWidth, y);
        pat->add_color_stop_rgb(0.0, 0.0, 0.0, 0.0);
        pat->add_color_stop_rgb(1.0, getColorR(rgb), getColorG(rgb), getColorB(rgb));

        double val = weight[rgb] / sum * (double)m_weightWidth;
        cr->rectangle(x, y, val, m_base);
        cr->set_source(pat);
        cr->fill();
    }
}

void
BinView::drawBottomGradient(const Cairo::RefPtr<Cairo::Context>& cr, Gtk::Allocation& allocation)
{
    const int alloc_width = allocation.get_width();
    Cairo::RefPtr<Cairo::LinearGradient> pat = Cairo::LinearGradient::create(0.0, 0.0, alloc_width, 0.0);
    pat->add_color_stop_rgb(0.0, 0.0, 0.0, 0.0);
    pat->add_color_stop_rgb(1.0, 1.0, 1.0, 1.0);
    cr->rectangle(0.0, allocation.get_height() - m_base + 1.0, alloc_width, allocation.get_height() - 2.0);
    cr->set_source(pat);
    cr->fill();
}

double
BinView::getColorR(uint32_t rgb)
{
    return rgb == 0 ? 1.0 : 0.0;
}

double
BinView::getColorG(uint32_t rgb)
{
    return rgb == 1 ? 1.0 : 0.0;
}

double
BinView::getColorB(uint32_t rgb)
{
    return rgb == 2 ? 1.0 : 0.0;
}
