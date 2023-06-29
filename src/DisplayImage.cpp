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



#include "DisplayImage.hpp"

DisplayImage::DisplayImage(Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
: Glib::ObjectBase(typeid(DisplayImage))
, Glib::Object()
, m_pixbuf{pixbuf}
{
}

Glib::RefPtr<DisplayImage>
DisplayImage::create(Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
	return Glib::RefPtr<DisplayImage>(new DisplayImage(pixbuf));
}

Glib::RefPtr<DisplayImage>
DisplayImage::crop(int x0, int y0, int width, int height)
{

	//std::cout << " crop"
	//	      << " x0 " << x0
	//	      << " width " << width
	//	      << " y0 " << y0
	//	      << " height " << height
	//	      << std::endl;
	Glib::RefPtr<DisplayImage> displayImage;
	if (m_pixbuf) {
		// create compat
		Glib::RefPtr<Gdk::Pixbuf> dest_pixbuf = Gdk::Pixbuf::create(
			m_pixbuf->get_colorspace(), m_pixbuf->get_has_alpha(), m_pixbuf->get_bits_per_sample(),
			width, height);
		m_pixbuf->copy_area(x0, y0, width, height, dest_pixbuf, 0, 0);
		displayImage = create(dest_pixbuf);
		//m_pixbuf->copy_options(dest_pixbuf); wont work as we set also on save ...
	}
	return displayImage;
}

Glib::RefPtr<Gdk::Pixbuf>
DisplayImage::getPixbuf()
{
	return m_pixbuf;
}

int DisplayImage::get_width()
{
	return m_pixbuf->get_width();
}

int DisplayImage::get_height()
{
	return m_pixbuf->get_height();
}

// no gtkmm function as it looks to see all options
std::map<Glib::ustring, Glib::ustring>
DisplayImage::getOptions()
{
	std::map<Glib::ustring, Glib::ustring> map;
	GHashTable *tbl = gdk_pixbuf_get_options(m_pixbuf.get()->gobj());
	if (tbl) {
		// less obstructing than g_hash_table_foreach(tbl, hashTblFunc, &map);
		GHashTableIter iter;
		g_hash_table_iter_init(&iter, tbl);
		gpointer key;
		gpointer value;
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			Glib::ustring sKey((const char *)key);
			Glib::ustring sValue((const char *)value);
			map.insert(std::make_pair(sKey, sValue));
		}
		g_hash_table_unref(tbl);
	}
	return map;
}