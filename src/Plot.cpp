/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf <gpl3@pfeifer-syscon.de>
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

namespace psc::ui {

void
PlotAxis::setMin(double min)
{
    m_min = min;
    m_dim = nonInitDim; // remember to calculate dependent
    m_factor = nonInitFactor;
}

double
PlotAxis::getMin()
{
    return m_min;
}

void
PlotAxis::setMax(double max)
{
    m_max = max;
    m_dim = nonInitDim; // remember to calculate dependent
    m_factor = nonInitFactor;
}

double
PlotAxis::getMax()
{
    return m_max;
}

double
PlotAxis::getDiff()
{
    return m_max - m_min;
}

double
PlotAxis::getStep()
{
    return getDiff() / static_cast<double>(m_pixel - 1);
}

double
PlotAxis::getFactor()
{

    if (m_factor == nonInitFactor) {
        m_factor = static_cast<double>(m_pixel - 1) / getDiff();
#       ifdef DEBUG
        std::cout << "PlotAxis::getFactor"
                  << " diff " << getDiff()
                  << " pixel " << m_pixel
                  << " factor " << m_factor << std::endl;
#       endif
    }
    return m_factor;
}

double
PlotAxis::getDim()
{
    if (m_dim == nonInitDim) {
        m_dim = std::floor(std::log10(getDiff()));
#       ifdef DEBUG
        std::cout << "PlotAxis::getDim"
                  << " diff " << getDiff()
                  << " dim " << m_dim << std::endl;
#       endif
    }
    return m_dim;
}

void
PlotAxis::scale()
{
    const auto scale = std::pow(10.0, getDim());
    m_min = std::floor(m_min / scale) * scale;
    m_max = std::ceil(m_max / scale) * scale;
    m_dim = nonInitDim;
    getDim();   // recompute dimension
    m_factor = nonInitFactor;
    getFactor();
}

double
PlotAxis::getGridStep()
{
    // for 0..100 the difference is 100 the dimension is 2
    //   so use 10 as grid step
    return std::pow(10.0, std::floor(std::log10(getDiff() - Plot::GRID_DIM)));
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
}

void
PlotAxis::setPixel(int pixel, int def)
{
    if (pixel <= 0) {
        m_pixel = def;
    }
    else {
        m_pixel = pixel;
    }
}


double
PlotAxis::toPixel(double x)
{
    auto pixel = (x - m_min) * getFactor();
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
    int dim = std::min(static_cast<int>(getDim()) - 1, 0);
    return Glib::ustring::sprintf("%%.%df", std::abs(dim));
}


PlotFunction::PlotFunction(double xMin, double xMax)
: PlotView()
{
    xAxis.setMin(xMin);
    xAxis.setMax(xMax);
}


std::array<double,2>
PlotFunction::computeMinMax()
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
PlotFunction::showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing)
{
    PlotAxis& yAxis = plotDrawing->getYAxis();
    const auto xGridDim = std::floor(std::log10(xAxis.getDiff()));
    auto xScale = std::pow(10.0, xGridDim);
    // use "rounded" grid positions
    auto xGridMin = std::floor(xAxis.getMin() / xScale) * xScale;
    auto xGridMax = std::ceil(xAxis.getMax() / xScale) * xScale;
    double xGridDiff = xGridMax - xGridMin;
    const auto xGridStep = std::pow(10.0, std::floor(std::log10(xGridDiff) - Plot::GRID_DIM));
#   ifdef DEBUG
    double xStep = xGridDiff / static_cast<double>(xAxis.getPixel() - 1);
    std::cout << "xGridStep " << xGridDim
              << " xScale " << xScale
              << " xGridMin " << xGridMin
              << " xGridMax " << xGridMax
              << " xDiff " << xGridDiff
              << " xStep " << xStep << std::endl;
#   endif
    for (double x = xGridMin; x <= xGridMax; x += xGridStep) {
        auto xPix = xAxis.toPixel(x);
#       ifdef DEBUG
        std::cout << "showX " << x << " pix " << xPix << std::endl;
#       endif
        ctx->set_source_rgb(plotDrawing->gridColor.get_red(), plotDrawing->gridColor.get_green(), plotDrawing->gridColor.get_blue());
        ctx->move_to(xPix, 0.0);
        ctx->line_to(xPix, yAxis.getPixel());
        ctx->stroke();
        ctx->set_source_rgb(plotDrawing->textColor.get_red(), plotDrawing->textColor.get_green(), plotDrawing->textColor.get_blue());
        ctx->move_to(xPix, yAxis.getPixel());
        ctx->show_text(Glib::ustring::sprintf(xAxis.getFormat(), x));
    }

}

void
PlotFunction::showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                          , PlotAxis& yAxis)
{
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

int
PlotFunction::getViewWidth(int displayWidth)
{
    xAxis.setPixel(displayWidth);
    return displayWidth;
}


PlotDiscrete::PlotDiscrete(const std::vector<double>& values)
: m_values{values}
{
}

std::array<double,2>
PlotDiscrete::computeMinMax()
{
    for (auto y : m_values) {
        m_yMin = std::min(m_yMin, y);
        m_yMax = std::max(m_yMax, y);
    }
    return std::array<double,2>{m_yMin, m_yMax};
}

void
PlotDiscrete::showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing)
{
    PlotAxis& yAxis= plotDrawing->getYAxis();
    for (size_t i = 0; i < m_values.size(); ++i) {
        auto str = getLabel(i);
        if (!str.empty()) {
            auto xPix = toXPixel(static_cast<double>(i));
            ctx->set_source_rgb(0.5, 0.5, 0.5);
            ctx->move_to(xPix, 0.0);
            ctx->line_to(xPix, yAxis.getPixel());
            ctx->stroke();
            ctx->set_source_rgb(1.0, 1.0, 1.0);
            ctx->move_to(xPix, yAxis.getPixel());
            ctx->show_text(str);
        }
    }
}

void
PlotDiscrete::showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                          , PlotAxis& yAxis)
{
    for (size_t n = 0; n < m_values.size(); ++n) {
        auto xPix = toXPixel(static_cast<double>(n));
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

int
PlotDiscrete::getViewWidth(int displayWidth)
{
    auto size = static_cast<int>(m_values.size());
    if (size > 0) {
        m_scale = static_cast<double>(displayWidth) / static_cast<double>(size);
        size = static_cast<int>(m_scale * size);
    }
    else {
        size = displayWidth;
    }
    return size;
}

int
PlotDiscrete::getPixel()
{
    auto size = static_cast<int>(m_values.size());
    size = static_cast<int>(m_scale * size);
    return size;
}


PlotDrawing::PlotDrawing(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::DrawingArea(cobject)
{
}

PlotDrawing::~PlotDrawing()
{
    if (m_func) {
        m_func->unsetPlotDrawing();
    }
}

bool
PlotDrawing::on_draw(const Cairo::RefPtr<::Cairo::Context>& cr)
{
    cr->set_source_rgb(0.0, 0.0, 0.3);
    if (!m_pixbuf
     || (m_func && m_func->getViewWidth(get_width()) != get_width())        // this will not refresh on just width-sizing for functions
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
PlotDrawing::setPlot(const std::shared_ptr<PlotView>& func)
{
    m_func = func;
    m_func->setPlotDrawing(this);
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

void
PlotDrawing::compute()
{
    yAxis.setPixel(get_height(), 480);
    yAxis.setInvertAxisMap(true);   // y is 0 a top
    //std::cout << "xMin " << xAxis.getMin()
    //          << " xMax " << xAxis.getMax()
    //          << " width " << xAxis.getPixel()
    //          << " xStep " << xAxis.getStep() << std::endl;
    auto yMinMax = m_func->computeMinMax();
    yAxis.setMin(yMinMax[0]);
    yAxis.setMax(yMinMax[1]);
#   ifdef DEBUG
    std::cout << "1. yMin " << yAxis.getMin()
              << " yMax " << yAxis.getMax() << std::endl;
#   endif
    yAxis.scale();
#   ifdef DEBUG
    std::cout << "dim " << yAxis.getDim()
              << " yMin " << yAxis.getMin()
              << " yMax " << yAxis.getMax() << std::endl;
    std::cout << "2. yDiff " << yAxis.getDiff()
              << " yGrid " << yAxis.getGridStep()
              << " yFact " << yAxis.getFactor() << std::endl;
#   endif
    int viewWidth = m_func->getViewWidth(get_width());
    m_pixbuf = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, viewWidth, yAxis.getPixel());
    auto ctx = Cairo::Context::create(m_pixbuf);
    ctx->set_source_rgb(backgroundColor.get_red(), backgroundColor.get_green(), backgroundColor.get_blue());
    ctx->rectangle(0.0, 0.0, viewWidth, yAxis.getPixel());
    ctx->fill();
    ctx->set_line_width(1.0);
    double x0 = m_func->getXMin();
    if (yAxis.getMin() < 0.0 && yAxis.getMax() > 0.0) {
        x0 = 0.0;
    }
    double xLbl = m_func->toXPixel(x0) + 3.0;
    ctx->set_source_rgb(gridColor.get_red(), gridColor.get_green(), gridColor.get_blue());
    const auto yGridStep{yAxis.getGridStep()};
    for (double y = yAxis.getMin(); y <= yAxis.getMax(); y += yGridStep) {
        auto yPix = yAxis.toPixel(y);
#       ifdef DEBUG
        std::cout << "showY " << y << " pix " << yPix << std::endl;
#       endif
        ctx->move_to(m_func->getPixel(), yPix);
        ctx->line_to(0.0, yPix);
        ctx->stroke();
        ctx->set_source_rgb(textColor.get_red(), textColor.get_green(), textColor.get_blue());
        auto yLbl = Glib::ustring::sprintf(yAxis.getFormat(), y);
        if (yPix <= 2.0) {
            Cairo::TextExtents textExtend;
            ctx->get_text_extents(yLbl, textExtend);
            yPix += textExtend.height;
        }
        else {
            yPix -= 2.0;
        }
        ctx->move_to(xLbl, yPix);
        ctx->show_text(yLbl);
    }
    m_func->showXGrid(ctx, this);
    ctx->set_source_rgb(plotColor.get_red(), plotColor.get_green(), plotColor.get_blue());
    m_func->showFunction(ctx, yAxis);
}

Plot::Plot(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
    builder->get_widget_derived<PlotDrawing>("drawing", m_drawing);
    builder->get_widget("active", m_active);
    m_active->set_active(m_drawing->isActive());
    m_active->signal_clicked().connect([this] {
        m_drawing->setActive(m_active->get_active());
    });
}

void
Plot::plot(const std::shared_ptr<PlotView>& func)
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

void
Plot::setPlotColor(Gdk::RGBA& plotColor)
{
    m_drawing->plotColor = plotColor;
}


void
Plot::on_response(int response)
{
    Gtk::Dialog::on_response(response);
}

void
Plot::show(Gtk::ApplicationWindow* sceneWindow, const std::shared_ptr<PlotView>& func)
{
    auto refBuilder = Gtk::Builder::create();
    try {
        auto appl = sceneWindow->get_application();
        refBuilder->add_from_resource(appl->get_resource_base_path() + "/plot-dlg.ui");
        Plot* plot;
        refBuilder->get_widget_derived("plotDlg", plot);
        if (plot) {
            plot->set_transient_for(*sceneWindow);
            plot->plot(func);
            plot->run();
            delete plot;
        }
        else {
            std::cerr << "Plot::show no plot-dlg" << std::endl;
        }
    }
    catch (const Glib::Error& ex) {
        std::cerr << "Plot::show error loading plot-dlg.ui " << ex.what() << std::endl;
    }
    return;
}

} /* namespace psc::ui */