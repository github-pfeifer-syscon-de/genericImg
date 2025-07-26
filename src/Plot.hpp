/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4;  coding: utf-8; -*-  */
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

#pragma once



#include <gtkmm.h>
#include <limits>

#include "config.h"


namespace psc::ui {

class PlotDrawing;

class PlotAxis
{
public:
    PlotAxis() = default;
    explicit PlotAxis(const PlotAxis& orig) = delete;
    virtual ~PlotAxis() = default;

    void setMin(double min);
    double getMin();
    void setMax(double max);
    double getMax();
    double getDiff();
    double getStep();   // to iterate values with pixel steps
    double getFactor(); // to convert values to pixel
    double getDim();    // dimension of difference e.g. 0..100 = 2
    void scale();       // round min,max to full values
    double getGridStep();
    int getPixel();
    void setPixel(int pixel);
    void setPixel(int pixel, int def);
    double toPixel(double x);
    void setInvertAxisMap(bool invertAxisMap); // e.g. for y-axis (0 at top)
    bool isInvertAxisMap();
    Glib::ustring getFormat();
protected:
    double m_min;
    double m_max;
    static constexpr auto nonInitFactor{1.0};
    double m_factor{nonInitFactor};
    int m_pixel{-1};
    static constexpr auto nonInitDim{std::numeric_limits<double>::lowest()};
    double m_dim{nonInitDim};
    bool m_invertAxisMap{false};
private:
};

class PlotView
{
public:
    PlotView() = default;
    explicit PlotView(const PlotView& orig) = delete;
    virtual ~PlotView() = default;

    virtual int getPixel() = 0;
    virtual std::array<double,2> computeMinMax() = 0;
    virtual void showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                 , PlotDrawing* plotDrawing) = 0;
    virtual void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& yAxis) = 0;
    virtual int getViewWidth(int displayWidth) = 0;
    virtual double getXMin() = 0;
    virtual double toXPixel(double x) = 0;
    void setPlotDrawing(PlotDrawing* plotDrawing)
    {
        m_plotDrawing = plotDrawing;
    }
    void unsetPlotDrawing()
    {
        m_plotDrawing = nullptr;
    }
protected:
    PlotDrawing* m_plotDrawing{nullptr};
};


class PlotFunction
: public PlotView
{
public:
    PlotFunction(double xMin, double xMax);
    explicit PlotFunction(const PlotFunction& orig) = delete;
    virtual ~PlotFunction() = default;

    virtual double calculate(double x) = 0;

    double getXMin() override
    {
        return xAxis.getMin();
    }
    double toXPixel(double x) override
    {
        return xAxis.toPixel(x);
    }

    std::array<double,2> computeMinMax() override;

    void showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                 , PlotDrawing* plotDrawing) override;
    void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& yAxis) override;
    int getViewWidth(int displayWidth) override;
protected:
    PlotAxis xAxis;
};

class PlotDiscrete
: public PlotView
{
public:
    PlotDiscrete(const std::vector<double>& values);
    explicit PlotDiscrete(const PlotDiscrete& orig) = delete;
    virtual ~PlotDiscrete() = default;

    // for positions where a label is desired return non empty string
    virtual Glib::ustring getLabel(size_t idx) = 0;
    std::array<double,2> computeMinMax() override;
    double getXMin() override
    {
        return 0.0;
    }
    double toXPixel(double x) override
    {
        return x * m_scale;
    }

    int getPixel() override;
    void showXGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                 , PlotDrawing* plotDrawing) override;
    void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& yAxis) override;
    int getViewWidth(int displayWidth) override;

protected:
    std::vector<double> m_values;
    double m_scale{1.0};
    // keep previous min / max
    double m_yMin{std::numeric_limits<double>::max()};
    double m_yMax{std::numeric_limits<double>::lowest()};

};

class PlotDrawing
: public Gtk::DrawingArea
{
public:
    PlotDrawing(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    explicit PlotDrawing(const PlotDrawing& orig) = delete;
    virtual ~PlotDrawing();

    void setPlot(const std::shared_ptr<PlotView>& func);
    void refresh();
    bool isActive()
    {
        return m_active;
    }
    void setActive(bool active)
    {
        m_active = active;
    }
    PlotAxis& getYAxis();
    Gdk::RGBA backgroundColor{"rgb(10%,10%,10%)"};
    Gdk::RGBA gridColor{"rgb(50%,50%,50%)"};
    Gdk::RGBA textColor{"rgb(100%,100%,100%)"};
    Gdk::RGBA plotColor{"rgb(0%,0%,100%)"};

protected:
    bool on_draw(const Cairo::RefPtr<::Cairo::Context>& cr) override;
    void compute();

    Cairo::RefPtr<Cairo::ImageSurface> m_pixbuf;

    PlotAxis yAxis;
    std::shared_ptr<PlotView> m_func;
    bool m_active{true};
};

class Plot
: public Gtk::Dialog
{
public:
    Plot(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    explicit Plot(const Plot& orig) = delete;
    virtual ~Plot() = default;

    void plot(const std::shared_ptr<PlotView>& func);
    static constexpr auto GRID_DIM{std::log10(2.0)};    // use for grid e.g. diff 1 Step 0.1, diff 2 step 1...
    void setBackground(Gdk::RGBA& background);
    void setGridColor(Gdk::RGBA& grid);
    void setTextColor(Gdk::RGBA& text);
    void setPlotColor(Gdk::RGBA& plot);

    static void show(Gtk::ApplicationWindow* sceneWindow, const std::shared_ptr<PlotView>& func);
protected:
    void on_response(int reponse);

private:
    PlotDrawing* m_drawing;
    Gtk::CheckButton* m_active;
};

} /* namespace psc::ui */