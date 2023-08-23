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

#include "ApplicationSupport.hpp"
#include "ImageList.hpp"

enum class ViewMode
{
    FIT,
    NATIVE
};

class BinView;
class ImageViewIntf;
class DisplayImage;

class ImageArea
: public Gtk::DrawingArea
{
public:
    ImageArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, ApplicationSupport& appSupport, ImageViewIntf* imageView);
    virtual ~ImageArea() = default;
    void setFile(const Glib::RefPtr<Gio::File> file);
    Glib::RefPtr<DisplayImage> getDisplayImage();
    ViewMode getViewMode();
    void setViewMode(ViewMode viewMode);
    void setPixbuf(Glib::RefPtr<DisplayImage> pixbuf);
    void setListStore(Glib::RefPtr<ImageList> imageList);
    void setSelected(bool selected);        // shows hide a selection rect
    Glib::RefPtr<Gdk::Cursor> mouse_pressed(double x, double y, bool pressed);  // notification about mouse movement
    void crop();
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    Glib::RefPtr<Gdk::Pixbuf> readPicture();
    void onNotifyLoad();
    void resetSelection();
    double getScale();
    void getOffset(double &xoffs, double &yoffs);
    void calculateView();
    void adjustScrollMin(Glib::RefPtr<Gtk::Adjustment> hScroll, double x);  // calculate scroll during selection
    void adjustScrollMax(Glib::RefPtr<Gtk::Adjustment> hScroll, double x, int alloc);  // calculate scroll during selection
    virtual void render(const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        const Glib::RefPtr<Gdk::Pixbuf> pixbuf);

    const int BORDER_SENSITIFTY = 10;

    Glib::RefPtr<Gio::File> m_file;
    Glib::RefPtr<DisplayImage> m_displayImage;
    Glib::RefPtr<Gdk::Pixbuf> m_scaledImage;
    Glib::Dispatcher m_drawDispatcher;
    std::future<Glib::RefPtr<Gdk::Pixbuf> > m_pictureReader;
    ApplicationSupport& m_appSupport;
    ViewMode m_viewMode{ViewMode::FIT};
    ImageViewIntf* m_imageView{nullptr};
    double x0{0.0},y0{0.0},x1{0.0},y1{0.0}; // selection in picture coords
    bool x0Move{false},y0Move{false},x1Move{false},y1Move{false};   // selection value dragged by mouse
};

