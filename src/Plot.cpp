/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf
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
#include <cmath>
#include <functional>

#include "Plot.hpp"
#include "config.h"

namespace psc::ui {

void
PlotAxis::setMinMax(double min, double max)
{
    m_min = min;
    m_max = max;
    adjust();
}

double
PlotAxis::getMin()
{
    return m_min;
}

double
PlotAxis::getMax()
{
    return m_max;
}

void
PlotAxis::adjust()
{
    m_dim = std::floor(std::log10(m_max - m_min));
    const auto roundScale = std::pow(10.0, m_dim - 1.0); // if we get diff 100 round to 10
    m_minScaled = std::floor(m_min / roundScale) * roundScale;
    m_maxScaled = std::ceil(m_max / roundScale) * roundScale;
    m_dim = std::floor(std::log10(m_maxScaled - m_minScaled));
    const auto gridScale = std::pow(10.0, m_dim); // if we get diff 110 round to 100
    m_gridMin = std::floor(m_minScaled / gridScale) * gridScale;
    m_gridMax = std::ceil(m_maxScaled / gridScale) * gridScale;
    m_gridStep = std::pow(10.0, std::floor(std::log10(m_maxScaled - m_minScaled) - Plot::GRID_DIM));
    m_step = (m_maxScaled - m_minScaled) / static_cast<double>(m_pixel - 1);
    m_factor = static_cast<double>(m_pixel - 1) / (m_maxScaled - m_minScaled);
#   ifdef DEBUG
    std::cout << "PlotAxis::adjust"
              << " diff " << m_max - m_min
              << " pixel " << m_pixel
              << " minScaled " << m_minScaled
              << " maxScaled " << m_maxScaled
              << " dim " << m_dim
              << " step " << m_step
              << " gridStep " << m_gridStep
              << " factor " << m_factor << std::endl;
#   endif
}


double
PlotAxis::getStep()
{
    return m_step;
}

int
PlotAxis::getPixel()
{
    return m_pixel;
}

void
PlotAxis::setPixel(int pixel)
{
    m_pixel = pixel;
    adjust();
}

void
PlotAxis::setPixel(int pixel, int def)
{
    if (pixel <= 0) {
        setPixel(def);
    }
    else {
        setPixel(pixel);
    }
}


double
PlotAxis::toPixel(double x)
{
    auto pixel = (x - m_minScaled) * m_factor;
    if (m_invertAxisMap) {
        pixel = m_pixel - pixel;
    }
    return pixel;
}

void
PlotAxis::setInvertAxisMap(bool invertAxisMap) // e.g. for y-axis (0 at top)
{
    m_invertAxisMap = invertAxisMap;
}

bool
PlotAxis::isInvertAxisMap()
{
    return m_invertAxisMap;
}

Glib::ustring
PlotAxis::getFormat()
{
    int dim = std::min(static_cast<int>(m_dim) - 1, 0);
    return Glib::ustring::sprintf("%%.%df", std::abs(dim));
}

void
PlotAxis::showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& yAxis)
{
    for (double x = m_gridMin; x <= m_gridMax; x += m_gridStep) {
        auto xPix = toPixel(x);
#       ifdef DEBUG
        std::cout << "PlotAxis::showXGrid " << x << " pix " << xPix << std::endl;
#       endif
        ctx->set_source_rgb(plotDrawing->gridColor.get_red(), plotDrawing->gridColor.get_green(), plotDrawing->gridColor.get_blue());
        ctx->move_to(xPix, 0.0);
        ctx->line_to(xPix, yAxis.getPixel());
        ctx->stroke();
        ctx->set_source_rgb(plotDrawing->textColor.get_red(), plotDrawing->textColor.get_green(), plotDrawing->textColor.get_blue());
        ctx->move_to(xPix, yAxis.getPixel());
        ctx->show_text(Glib::ustring::sprintf(getFormat(), x));
    }
}

void
PlotAxis::showDiscrete(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& yAxis
                       , const std::shared_ptr<PlotDiscrete>& discrete)
{
    for (size_t n = 0; n < discrete->getValuesSize(); ++n) {
        auto lbl = discrete->getLabel(n);
        if (!lbl.empty()) {
            auto xPix = toPixel(static_cast<double>(n));
            ctx->set_source_rgb(plotDrawing->gridColor.get_red(), plotDrawing->gridColor.get_green(), plotDrawing->gridColor.get_blue());
            ctx->move_to(xPix, 0.0);
            ctx->line_to(xPix, yAxis.getPixel());
            ctx->stroke();
            ctx->set_source_rgb(plotDrawing->textColor.get_red(), plotDrawing->textColor.get_green(), plotDrawing->textColor.get_blue());
            ctx->move_to(xPix, yAxis.getPixel());
            ctx->show_text(lbl);
        }
    }
}

void
PlotAxis::showYGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& xAxis)
{
    double x0{xAxis.getMin()};
    if (getMin() < 0.0 && getMax() > 0.0) {
        x0 = 0.0;
    }
    double xLbl = xAxis.toPixel(x0) + 3.0;
    for (double y = m_gridMin; y <= m_gridMax; y += m_gridStep) {
        auto yPix = toPixel(y);
#       ifdef DEBUG
        std::cout << "PlotAxis::showYGrid " << y << " pix " << yPix << std::endl;
#       endif
        ctx->set_source_rgb(plotDrawing->gridColor.get_red(), plotDrawing->gridColor.get_green(), plotDrawing->gridColor.get_blue());
        ctx->move_to(xAxis.getPixel(), yPix);
        ctx->line_to(0.0, yPix);
        ctx->stroke();
        ctx->set_source_rgb(plotDrawing->textColor.get_red(), plotDrawing->textColor.get_green(), plotDrawing->textColor.get_blue());
        auto yLbl = Glib::ustring::sprintf(getFormat(), y);
        if (yPix <= 2.0) {
            Cairo::TextExtents textExtents;
            ctx->get_text_extents(yLbl, textExtents);
            yPix += textExtents.height;
        }
        else {
            yPix -= 2.0;
        }
        ctx->move_to(xLbl, yPix);
        ctx->show_text(yLbl);
    }
}

void
PlotView::setPlotDrawing(PlotDrawing* plotDrawing)
{
    m_plotDrawing = plotDrawing;
}

void
PlotView::unsetPlotDrawing()
{
    m_plotDrawing = nullptr;
}

void
PlotView::setPlotColor(Gdk::RGBA plotColor)
{
    m_plotColor = plotColor;
}

PlotFunction::PlotFunction()
: PlotView()
{
}


std::array<double,2>
PlotFunction::computeMinMax(PlotAxis& xAxis)
{
    auto yMin = std::numeric_limits<double>::max();
    auto yMax = std::numeric_limits<double>::lowest();
    double x = xAxis.getMin();
    for (int32_t n = 0; n < xAxis.getPixel(); ++n) {
        auto y = calculate(x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
        x += xAxis.getStep();
    }
    return std::array<double,2>{yMin, yMax};
}

void
PlotFunction::showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                          , PlotAxis& xAxis
                          , PlotAxis& yAxis)
{
    ctx->set_source_rgb(m_plotColor.get_red(), m_plotColor.get_green(), m_plotColor.get_blue());
    double x = xAxis.getMin();
    for (int32_t n = 0; n < xAxis.getPixel(); ++n) {
        auto y = calculate(x);
        auto xPix = xAxis.toPixel(x);
        auto yPix = yAxis.toPixel(y);
        if (n == 0) {
            ctx->move_to(xPix, yPix);
        }
        else {
            ctx->line_to(xPix, yPix);
        }
        x += xAxis.getStep();
    }
    ctx->stroke();
}


PlotDiscrete::PlotDiscrete(const std::vector<double>& values)
: m_values{values}
{
}

std::array<double,2>
PlotDiscrete::computeMinMax(PlotAxis& xAxis)
{
    for (auto y : m_values) {
        m_yMin = std::min(m_yMin, y);
        m_yMax = std::max(m_yMax, y);
    }
    return std::array<double,2>{m_yMin, m_yMax};
}

void
PlotDiscrete::showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                          , PlotAxis& xAxis
                          , PlotAxis& yAxis)
{
    ctx->set_source_rgb(m_plotColor.get_red(), m_plotColor.get_green(), m_plotColor.get_blue());
    for (size_t n = 0; n < m_values.size(); ++n) {
        auto xPix = xAxis.toPixel(static_cast<double>(n));
        auto y = m_values[n];
        auto yPix = yAxis.toPixel(y);
        if (n == 0) {
            ctx->move_to(xPix, yPix);
        }
        else {
            ctx->line_to(xPix, yPix);
        }
    }
    ctx->stroke();
}

size_t
PlotDiscrete::getValuesSize()
{
    return m_values.size();
}

//int
//PlotDiscrete::getViewWidth(int displayWidth)
//{
//    auto size = static_cast<int>(m_values.size());
//    if (size > 0) {
//        m_scale = static_cast<double>(displayWidth) / static_cast<double>(size);
//        size = static_cast<int>(m_scale * size);
//    }
//    else {
//        size = displayWidth;
//    }
//    return size;
//}
//
//int
//PlotDiscrete::getPixel()
//{
//    auto size = static_cast<int>(m_values.size());
//    size = static_cast<int>(m_scale * size);
//    return size;
//}
//

PlotDrawing::PlotDrawing(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::DrawingArea(cobject)
{
}

PlotDrawing::~PlotDrawing()
{
    for (auto& func : m_func) {
        if (func) {
            func->unsetPlotDrawing();
        }
    }
}

bool
PlotDrawing::on_draw(const Cairo::RefPtr<::Cairo::Context>& cr)
{
    //cr->set_source_rgb(0.0, 0.0, 0.3);
    if (!m_pixbuf
     || xAxis.getPixel() != get_width()        // this will not refresh on just width-sizing for functions
     || yAxis.getPixel() != get_height()) {
        compute();
    }
    if (m_pixbuf) {
        cr->set_source(m_pixbuf, 0.0, 0.0);
    }
    cr->rectangle(0.0, 0.0, get_width(), get_height());
    cr->fill();

    return true;
}

void
PlotDrawing::setPlot(const std::vector<std::shared_ptr<PlotView>>& func)
{
    m_func = func;
    for (auto& fun : func) {
        fun->setPlotDrawing(this);
    }
}

void
PlotDrawing::refresh()
{
    compute();
    queue_draw();
}

PlotAxis&
PlotDrawing::getYAxis()
{
    return yAxis;
}

PlotAxis&
PlotDrawing::getXAxis()
{
    return xAxis;
}


void
PlotDrawing::compute()
{
    yAxis.setPixel(get_height(), 480);
    xAxis.setPixel(get_width(), 320);
    yAxis.setInvertAxisMap(true);   // as otherwise y starts from top
    //std::cout << "xMin " << xAxis.getMin()
    //          << " xMax " << xAxis.getMax()
    //          << " width " << xAxis.getPixel()
    //          << " xStep " << xAxis.getStep() << std::endl;
    auto yMin{std::numeric_limits<double>::max()};
    auto yMax{std::numeric_limits<double>::lowest()};
    for (auto& func : m_func) {
        auto yMinMax = func->computeMinMax(xAxis);
        yMin = std::min(yMin, yMinMax[0]);
        yMax = std::max(yMax, yMinMax[1]);
    }
    yAxis.setMinMax(yMin, yMax);
#   ifdef DEBUG
    std::cout << "1. yMin " << yAxis.getMin()
              << " yMax " << yAxis.getMax() << std::endl;
#   endif
#   ifdef DEBUG
    std::cout << "dim " << yAxis.getDim()
              << " yMin " << yAxis.getMin()
              << " yMax " << yAxis.getMax() << std::endl;
    std::cout << "2. yDiff " << yAxis.getDiff()
              << " yGrid " << yAxis.getGridStep()
              << " yFact " << yAxis.getFactor() << std::endl;
#   endif
    int viewWidth = 120;
    viewWidth = std::max(viewWidth, xAxis.getPixel());
    m_pixbuf = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, viewWidth, yAxis.getPixel());
    auto ctx = Cairo::Context::create(m_pixbuf);
    ctx->set_source_rgb(backgroundColor.get_red(), backgroundColor.get_green(), backgroundColor.get_blue());
    ctx->rectangle(0.0, 0.0, viewWidth, yAxis.getPixel());
    ctx->fill();
    ctx->set_line_width(1.0);

    yAxis.showYGrid(ctx, this, xAxis);
    bool shownXaxis{false};
    for (auto& func : m_func) {
        auto discrete = std::dynamic_pointer_cast<PlotDiscrete>(func);
        if (!shownXaxis && discrete) {
            xAxis.showDiscrete(ctx, this, yAxis, discrete);
            shownXaxis = true;
            break;
        }
    }
    if (!shownXaxis) {
        xAxis.showXGrid(ctx, this, yAxis);
    }
    for (auto& func : m_func) {
        func->showFunction(ctx, xAxis, yAxis);
    }
}

Plot::Plot(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
    builder->get_widget_derived<PlotDrawing>("drawing", m_drawing);
}

void
Plot::plot(const std::vector<std::shared_ptr<PlotView>>& func)
{
    m_drawing->setPlot(func);
}

void
Plot::setBackground(Gdk::RGBA& background)
{
    m_drawing->backgroundColor = background;
}

void
Plot::setGridColor(Gdk::RGBA& grid)
{
    m_drawing->gridColor = grid;
}

void
Plot::setTextColor(Gdk::RGBA& text)
{
    m_drawing->textColor = text;
}


} /* namespace psc::ui */
