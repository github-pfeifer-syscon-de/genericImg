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

#include "ImageArea.hpp"
#include "DisplayImage.hpp"
#include "ImageView.hpp"

ImageArea::ImageArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder
        , ApplicationSupport& applicationSupport, ImageViewIntf *imageView)
: Gtk::DrawingArea(cobject)
, m_file()
, m_displayImage()
, m_drawDispatcher()
, m_appSupport{applicationSupport}
, m_imageView{imageView}
{
   m_drawDispatcher.connect(sigc::mem_fun(*this, &ImageArea::onNotifyLoad));
   Gdk::RGBA color("#000");
   override_color(color);
}


Glib::RefPtr<Gdk::Pixbuf>
ImageArea::readPicture()
{
    finally notify_done([&] {
        m_drawDispatcher.emit();
    });
    Glib::RefPtr<Gdk::Pixbuf> pixbuf ;
    try {
        pixbuf = Gdk::Pixbuf::create_from_file(m_file->get_path());
    }
    catch (const Glib::Exception& ex) {
        m_appSupport.showError(ex.what());  // for glib:error what seems sufficient
    }
    return pixbuf;
}

void
ImageArea::setFile(const Glib::RefPtr<Gio::File> file)
{
    m_file = file;
    m_displayImage.clear();       // remove previous reference, matters if load will not succeed
    m_pictureReader = std::async(std::launch::async, &ImageArea::readPicture, this);
}

Glib::RefPtr<DisplayImage>
ImageArea::getDisplayImage()
{
    return m_displayImage;
}

void
ImageArea::resetSelection()
{
    x0 = 0.0;
    y0 = 0.0;
    x1 = 0.0;
    y1 = 0.0;
	x0Move = false;
	y0Move = false;
	x1Move = false;
	y1Move = false;
}

void
ImageArea::setPixbuf(Glib::RefPtr<DisplayImage> displayImage)
{
    //std::cout << "ImageArea::setPixbuf" << std::endl;
    m_scaledImage.reset();
    m_displayImage = displayImage;
    m_imageView->updateImageInfos(displayImage);
    resetSelection();
	calculateView();
}

void
ImageArea::calculateView()
{
    gint width;
    gint height;
    Gtk::Viewport* view = dynamic_cast<Gtk::Viewport *>(get_parent());
    if (view) {
        switch (m_viewMode) {
            case ViewMode::FIT:
                width = view->get_allocated_width();
                height = view->get_allocated_height();
                break;
            case ViewMode::NATIVE:
            default:
                width = m_displayImage->get_width();
                height = m_displayImage->get_height();
                break;
        }
        //std::cout << "Mode " << (int)m_viewMode
        //          << " widht " << width
        //          << " height " << height
        //          << std::endl;
        set_size_request(width, height);
    }
    else {
        std::cerr << "ImageArea::setPixbuf parent no viewport ..." << std::endl;
    }
}

void
ImageArea::onNotifyLoad()
{
    try {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = m_pictureReader.get();
  		Glib::RefPtr<DisplayImage> displ = DisplayImage::create(pixbuf);
        setPixbuf(displ);
    }
    catch (const Glib::Error& ex) {         // this catches error on loading image
        m_appSupport.showError(ex.what());  // for glib:error what seems sufficient
    }
    queue_draw();
}

void
ImageArea::setSelected(bool selected)
{
    if (selected) {
        if (m_displayImage) {
            //double xoffs,yoffs;
            //getOffset(xoffs, yoffs);
			//double scale = getScale();
            const double img_width = (double)m_displayImage->get_width();
            const double img_height = (double)m_displayImage->get_height();
			const double borderWidth = img_width / 10.0;
			const double borderHeight = img_height / 10.0;
            x0 = borderWidth;
            y0 = borderHeight;
            x1 = img_width - borderWidth;
            y1 = img_height - borderHeight;
			//std::cout << "setSelected x0 " << x0
			//		  << " y0 " << y0
			//	      << " x1 " << x1
			//	      << " y1 " << y1
			//	      << std::endl;
        }
    }
    else {
        resetSelection();
    }
    queue_draw();
}

void
ImageArea::adjustScrollMin(Glib::RefPtr<Gtk::Adjustment> scroll, double x)
{
	if (scroll
	 && scroll->get_value() > scroll->get_lower()
	 && x < scroll->get_minimum_increment()) {
		scroll->set_value(std::max(scroll->get_lower(), scroll->get_value() - scroll->get_minimum_increment()));
	}
}

void
ImageArea::adjustScrollMax(Glib::RefPtr<Gtk::Adjustment> scroll, double x, int alloc)
{
	if (scroll
	 && scroll->get_value() < scroll->get_upper() - scroll->get_page_size()
	 && x > alloc - scroll->get_minimum_increment()) {
		scroll->set_value(std::min(scroll->get_upper() - scroll->get_page_size(), scroll->get_value() + scroll->get_minimum_increment()));
	}

}

Glib::RefPtr<Gdk::Cursor>
ImageArea::mouse_pressed(double x, double y, bool drag)
{
	//std::cout << "mouse_pressed x " << x
	//         << " y " << y
	//         << " drag " << (drag ? "yes" : "no") << std::endl;
    Glib::RefPtr<Gdk::Cursor> cursor;
	double scale = getScale();
	double xoffs,yoffs;
	getOffset(xoffs, yoffs);
	Glib::RefPtr<Gtk::Adjustment> hScroll;
	Glib::RefPtr<Gtk::Adjustment> vScroll;
	int alloc_width = 0, alloc_height = 0;
	Gtk::Viewport* view = dynamic_cast<Gtk::Viewport *>(get_parent());
	if (view) {
		Gtk::ScrolledWindow* scrolled = dynamic_cast<Gtk::ScrolledWindow *>(view->get_parent());
		if (scrolled) {
			hScroll = scrolled->get_hadjustment();
			xoffs -= hScroll->get_value();
			vScroll = scrolled->get_vadjustment();
			yoffs -= vScroll->get_value();
			alloc_width = scrolled->get_allocated_width();
			alloc_height = scrolled->get_allocated_height();
		}
	}
	double xp = std::max(0.0, (x - xoffs) / scale);	// to picture coords
	double yp = std::max(0.0, (y - yoffs) / scale);
	if (m_displayImage) {
		xp = std::min((double)m_displayImage->get_width(), xp);
		yp = std::min((double)m_displayImage->get_height(), yp);
	}
	if ((x0Move || y0Move || x1Move || y1Move)
	 && drag) {
		if (x0Move
	     &&	xp < x1 - BORDER_SENSITIFTY) {
			x0 = xp;
			adjustScrollMin(hScroll, x);
			queue_draw();
		}
		if (x1Move
		 && xp >= x0 + BORDER_SENSITIFTY) {
			x1 = xp;
			adjustScrollMax(hScroll, x, alloc_width);
			queue_draw();
		}
		if (y0Move
		 && yp < y1 - BORDER_SENSITIFTY) {
			y0 = yp;
			adjustScrollMin(vScroll, y);
			queue_draw();
		}
		if (y1Move
		 && yp >= y0 + BORDER_SENSITIFTY) {
			y1 = yp;
			adjustScrollMax(vScroll, y, alloc_height);
			queue_draw();
		}
	}
	else {
		x0Move = false;
		x1Move = false;
		y0Move = false;
		y1Move = false;
		if ((std::abs(x0 - xp) <= BORDER_SENSITIFTY)
		 && yp >= y0 && yp <= y1) {
			x0Move = true;
		}
		else if ((std::abs(x1 - xp) <= BORDER_SENSITIFTY)
		 && yp >= y0 && yp <= y1) {
			x1Move = true;
		}
		if ((std::abs(y0 - yp) <= BORDER_SENSITIFTY)
		 && xp >= x0 && xp <= x1) {
			y0Move = true;
		}
		else if ((std::abs(y1 - yp) <= BORDER_SENSITIFTY)
		 && xp >= x0 && xp <= x1) {
			y1Move = true;
		}
	}
	Gdk::CursorType cursorType = Gdk::ARROW;
	if (x0Move) {
		if (y0Move) {
			cursorType =  Gdk::TOP_LEFT_CORNER;
		}
		else if (y1Move) {
			cursorType = Gdk::BOTTOM_LEFT_CORNER;
		}
		else {
			cursorType = Gdk::LEFT_SIDE;
		}
	}
	else if (x1Move) {
		if (y0Move) {
			cursorType =  Gdk::TOP_RIGHT_CORNER;
		}
		else if (y1Move) {
			cursorType = Gdk::BOTTOM_RIGHT_CORNER;
		}
		else {
			cursorType = Gdk::RIGHT_SIDE;
		}
	}
	else if (y0Move) {
		cursorType = Gdk::TOP_SIDE;
	}
	else if (y1Move) {
		cursorType = Gdk::BOTTOM_SIDE;
	}
	if (cursorType != Gdk::ARROW) {
		cursor = Gdk::Cursor::create(get_display(), cursorType);
	}
    return cursor;
}

void
ImageArea::crop()
{
	int width = static_cast<int>(x1 - x0);
	int height  = static_cast<int>(y1 - y0);
	if (width <= 0
	 || height <= 0) {
		std::cerr << "DisplayImage::crop"
			     << " width " << width
		         << " height " << height
		         << " allows no selection."
		         << std::endl;
		return;
	}
	Glib::RefPtr<DisplayImage> dest = m_displayImage->crop(
            static_cast<int>(x0), static_cast<int>(y0)
            , width, height);
	setPixbuf(dest);
	queue_draw();

}

double
ImageArea::getScale()
{
    double scale = 1.0;
    Gtk::Allocation allocation = get_allocation();
    const int alloc_width = allocation.get_width();
    const int alloc_height = allocation.get_height();
    if (m_displayImage) {
        const int img_width = m_displayImage->get_width();
        const int img_height = m_displayImage->get_height();
        double xscale = (double) alloc_width / (double) img_width;
        double yscale = (double) alloc_height  / (double) img_height;
        //std::cout << "xscale " << xscale << " yscale " <<  yscale << std::endl;
        scale = std::min(xscale, yscale);
    }
    return scale;
}

void
ImageArea::getOffset(double &xoffs, double &yoffs)
{
    Gtk::Allocation allocation = get_allocation();
    const int alloc_width = allocation.get_width();
    const int alloc_height = allocation.get_height();
    if (m_displayImage) {
        double scale = getScale();
        const int img_width = m_displayImage->get_width();
        const int img_height = m_displayImage->get_height();
        xoffs = (alloc_width - scale * img_width) / 2.0;
        yoffs = (alloc_height - scale * img_height) / 2.0;
        return;
    }
    xoffs = 0.0;
    yoffs = 0.0;
}

void
ImageArea::render(const Cairo::RefPtr<Cairo::Context>& cairoCtx,
    const Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    Gdk::Cairo::set_source_pixbuf(cairoCtx, pixbuf, 0, 0);
    cairoCtx->rectangle(0, 0, pixbuf->get_width(), pixbuf->get_height());
    cairoCtx->fill();
}


bool
ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::DrawingArea::on_draw(cr);
    if (m_displayImage
     && m_displayImage->getPixbuf()) {
        double scale = getScale();
        double xoffs,yoffs;
        getOffset(xoffs, yoffs);
        cr->translate(xoffs, yoffs);    // center
        //cr->scale(scale, scale);
        //Gdk::Cairo::set_source_pixbuf(cr, m_displayImage->getPixbuf(), 0, 0);
        //cr->set_operator(Cairo::OPERATOR_SOURCE);
        //cr->paint();
        // prefer some cached scaled instance
        auto pixbuf = m_displayImage->getPixbuf();
        int scaledWidth = static_cast<int>(static_cast<double>(pixbuf->get_width()) * scale);
        int scaledHeight = static_cast<int>(static_cast<double>(pixbuf->get_height()) * scale);
        if (!m_scaledImage
         || (std::abs(scaledWidth - m_scaledImage->get_width()) > 10
         &&  std::abs(scaledHeight - m_scaledImage->get_height()) > 10)) {   // scale with steps not every pixel
            //std::cout << "scaling "
            //          << " width " << scaledWidth
            //          << " height " << scaledHeight << std::endl;
            m_scaledImage = pixbuf->scale_simple(scaledWidth, scaledHeight, Gdk::InterpType::INTERP_BILINEAR);
        }
        render(cr, m_scaledImage);
        if (x1 > x0 && y1 > y0) {   // this is only temporary
            //cr->reset_clip();
            //
            cr->begin_new_path();
            cr->move_to(x0, y0);
            cr->line_to(x1, y0);
            cr->line_to(x1, y1);
            cr->line_to(x0, y1);
            cr->close_path();
			cr->set_source_rgb(1.0, 1.0, 1.0);
			cr->set_line_width(1.0/scale);
            cr->stroke();
            //cr->paint_with_alpha(0.6);     // show shaded
            //cr->reset_clip();
        }
    }
    return true;
}

ViewMode
ImageArea::getViewMode()
{
    return m_viewMode;
}

void
ImageArea::setViewMode(ViewMode viewMode)
{
    m_viewMode = viewMode;
    if (m_displayImage) {
		calculateView();		// change view
        queue_draw();           // and display
    }
}
