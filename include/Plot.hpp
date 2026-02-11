/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4;  coding: utf-8; -*-  */
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

#pragma once



#include <gtkmm.h>
#include <limits>
#include <vector>


namespace psc::ui {

class PlotDrawing;
class PlotDiscrete;
class PlotAxis;

class PlotGrid
{
public:
    PlotGrid() = default;
    explicit PlotGrid(const PlotGrid& orig) = delete;
    virtual ~PlotGrid() = default;

    virtual void showGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& majorAxis
                       , PlotAxis& minorAxis) = 0;
};

class PlotGridX
: public PlotGrid
{
public:
    PlotGridX() = default;
    explicit PlotGridX(const PlotGridX& orig) = delete;
    virtual ~PlotGridX() = default;

    void showGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& majorAxis
                       , PlotAxis& minorAxis) override;
};

class PlotGridY
: public PlotGrid
{
public:
    PlotGridY() = default;
    explicit PlotGridY(const PlotGridY& orig) = delete;
    virtual ~PlotGridY() = default;

    void showGrid(const Cairo::RefPtr<Cairo::Context>& ctx
                       , PlotDrawing* plotDrawing
                       , PlotAxis& majorAxis
                       , PlotAxis& minorAxis) override;
};


class PlotAxis
{
public:
    PlotAxis(std::shared_ptr<PlotGrid> grid);
    explicit PlotAxis(const PlotAxis& orig) = delete;
    virtual ~PlotAxis() = default;

    void setMinMax(double min, double max);
    double getMin();
    double getMax();
    double getStep();
    int getPixel();
    void setPixel(int pixel);
    void setPixel(int pixel, int def);
    double toPixel(double x);
    void setInvertAxisMap(bool invertAxisMap); // e.g. for y-axis (0 at top)
    bool isInvertAxisMap();
    Glib::ustring getFormat();
    double getGridMin();
    double getGridMax();
    double getGridStep();
    std::shared_ptr<PlotGrid> getGrid();
    void setGrid(std::shared_ptr<PlotGrid> grid);

protected:
    void adjust();
    double m_min{-1.0};
    double m_max{1.0};
    double m_minScaled{-1.0};
    double m_maxScaled{1.0};
    double m_gridMin{-1.0};
    double m_gridMax{1.0};
    double m_factor{1.0};
    int m_pixel{100};
    double m_dim{1.0};
    double m_step{1.0};
    double m_gridStep{1.0};
    bool m_invertAxisMap{false};
    std::shared_ptr<PlotGrid> m_grid;
private:
};

class PlotView
{
public:
    PlotView() = default;
    explicit PlotView(const PlotView& orig) = delete;
    virtual ~PlotView() = default;

    virtual std::array<double,2> computeMinMax(PlotAxis& xAxis) = 0;
    virtual void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& xAxis
                    , PlotAxis& yAxis) = 0;
    void setPlotDrawing(PlotDrawing* plotDrawing);
    void unsetPlotDrawing();
    void setPlotColor(Gdk::RGBA plotColor);
protected:
    PlotDrawing* m_plotDrawing{nullptr};
    Gdk::RGBA m_plotColor{"rgb(0%,0%,100%)"};
};


class PlotFunction
: public PlotView
{
public:
    PlotFunction();
    explicit PlotFunction(const PlotFunction& orig) = delete;
    virtual ~PlotFunction() = default;

    virtual double calculate(double x) = 0;
    std::array<double,2> computeMinMax(PlotAxis& xAxis) override;

    void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& xAxis
                    , PlotAxis& yAxis);
protected:
};

class PlotDiscrete
: public PlotView
{
public:
    PlotDiscrete(const std::vector<double>& values);
    explicit PlotDiscrete(const PlotDiscrete& orig) = delete;
    virtual ~PlotDiscrete() = default;

    // for positions where a label is desired return non empty string
    //virtual Glib::ustring getLabel(size_t idx) = 0;
    std::array<double,2> computeMinMax(PlotAxis& xAxis) override;
    void showFunction(const Cairo::RefPtr<Cairo::Context>& ctx
                    , PlotAxis& xAxis
                    , PlotAxis& yAxis) override;
    size_t getValuesSize();
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

    void setPlot(const std::vector<std::shared_ptr<PlotView>>& func);
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
    PlotAxis& getXAxis();
    Gdk::RGBA backgroundColor{"rgb(10%,10%,10%)"};
    Gdk::RGBA gridColor{"rgb(50%,50%,50%)"};
    Gdk::RGBA textColor{"rgb(100%,100%,100%)"};
    double getXMin()
    {
        return xAxis.getMin();
    }
    double toXPixel(double x)
    {
        return xAxis.toPixel(x);
    }

protected:
    bool on_draw(const Cairo::RefPtr<::Cairo::Context>& cr) override;
    void compute();

    Cairo::RefPtr<Cairo::ImageSurface> m_pixbuf;

    PlotAxis yAxis;
    PlotAxis xAxis;
    std::vector<std::shared_ptr<PlotView>> m_func;
    bool m_active{true};
};

class Plot
: public Gtk::Dialog
{
public:
    Plot(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    explicit Plot(const Plot& orig) = delete;
    virtual ~Plot() = default;

    void plot(const std::vector<std::shared_ptr<PlotView>>& func);
    static constexpr auto GRID_DIM{std::log10(2.0)};    // use for grid e.g. diff 1 Step 0.1, diff 2 step 1...
    void setBackground(Gdk::RGBA& background);
    void setGridColor(Gdk::RGBA& grid);
    void setTextColor(Gdk::RGBA& text);
    void setPlotColor(Gdk::RGBA& plot);

protected:

    PlotDrawing* m_drawing;
};

} /* namespace psc::ui */
