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
#include <sys/stat.h>
#include <stdlib.h>

#include "ImageList.hpp"
#include "ExifReader.hpp"
#include "DisplayImage.hpp"

VariableColumns ImageList::m_variableColumns;

ImageList::ImageList()
: Gtk::TreeStore(m_variableColumns)
{
}


// Attributes
//File space2001.jpg type 1
//   standard::type=1
//   standard::name=space2001.jpg
//   standard::display-name=space2001.jpg
//   standard::edit-name=space2001.jpg
//   standard::copy-name=space2001.jpg
//   standard::icon=GThemedIcon:0x1afb850
//   standard::content-type=image/jpeg
//   standard::fast-content-type=image/jpeg
//   standard::size=907425
//   standard::allocated-size=909312
//   standard::symbolic-icon=GThemedIcon:0x1afb1a8
//   etag::value=1610465944:281733
//   id::file=l45827:2117186
//   id::filesystem=l45827
//   access::can-read=TRUE
//   access::can-write=TRUE
//   access::can-execute=FALSE
//   access::can-delete=TRUE
//   access::can-trash=TRUE
//   access::can-rename=TRUE
//   time::modified=1610465944
//   time::modified-usec=281733
//   time::access=1612284863
//   time::access-usec=281970
//   time::changed=1610465944
//   time::changed-usec=351732
//   time::created=1610465944
//   time::created-usec=251733
//   unix::device=45827
//   unix::inode=2117186
//   unix::mode=33188
//   unix::nlink=1
//   unix::uid=1000
//   unix::gid=1000
//   unix::rdev=0
//   unix::block-size=4096
//   unix::blocks=1776
//   owner::user=rpf
//   owner::user-real=
//   owner::group=rpf
//   thumbnail::path=/home/rpf/.cache/thumbnails/large/9d4e70442b728f0e56e87bdf95fe994b.png
//   thumbnail::is-valid=TRUE
//   metadata::download-uri=https://img.wallpapersafari.com/desktop/1920/1080/49/15/dh8Rp6.jpg
void
ImageList::fillList(const Glib::RefPtr<Gio::File> file)
{
    clear();
	auto chlds = appendList("File", "");

    Glib::ustring path(file->get_parent()->get_path());
    appendList(chlds, "Path", path);

    Glib::RefPtr<Gio::FileInfo> info = file->query_info("standard::*,time::*,owner::*,unix::*,metadata::*");
    Glib::ustring name = info->get_display_name();
    appendList(chlds, "Name", name);

    Glib::ustring content = info->get_content_type();
    appendList(chlds, "Content", content);

    guint64 uModified = info->get_attribute_uint64("time::modified");
    Glib::DateTime modified = Glib::DateTime::create_now_local(uModified);
    appendList(chlds, "Modified", modified);

    guint64 uCreated = info->get_attribute_uint64("time::created");
    Glib::DateTime created = Glib::DateTime::create_now_local(uCreated);
    appendList(chlds, "Created", created);

    goffset uSize = info->get_size();
    Glib::ustring size(formatScale(uSize)); // , "Bi"
    appendList(chlds, "Size", size);

    Glib::ustring user = Glib::ustring::sprintf("%s %d %s %d"
          , info->get_attribute_string("owner::user")
          , info->get_attribute_uint32("unix::uid")
          , info->get_attribute_string("owner::group")
          , info->get_attribute_uint32("unix::gid"));
    appendList(chlds, "User,Group", user);

    guint32 mode = info->get_attribute_uint32("unix::mode");
    Glib::ustring perm = Glib::ustring::sprintf("-%s%s%s%s%s%s%s%s%s %04o"
          , mode & S_IRUSR ? "r" : "-"
          , mode & S_IWUSR ? "w" : "-"
          , mode & S_IXUSR ? "x" : "-"
          , mode & S_IRGRP ? "r" : "-"
          , mode & S_IWGRP ? "w" : "-"
          , mode & S_IXGRP ? "x" : "-"
          , mode & S_IROTH ? "r" : "-"
          , mode & S_IWOTH ? "w" : "-"
          , mode & S_IXOTH ? "x" : "-"
          , mode & 0x01ff);
    appendList(chlds, "Permissions", perm);

    auto metas = info->list_attributes("metadata");
    for (auto meta : metas) {
        Glib::ustring metakey(meta);
        metakey.replace(0, 10, ""); // cut metadata::
        Glib::ustring metaval(info->get_attribute_as_string(meta));
        appendList(chlds, metakey.c_str(), metaval);
    }
}

static int
findGCD(int a, int b)
{
    if(b == 0)
      return a;
   return findGCD(b, a % b);
}

void
ImageList::fillList(Glib::RefPtr<DisplayImage>& pixbuf)
{
	auto chlds = appendList("Image", "");
    Glib::ustring image = Glib::ustring::sprintf("%d * %d"
          , pixbuf->get_width()
          , pixbuf->get_height());

    int a = std::max(pixbuf->get_width(), pixbuf->get_height());
    int b = std::min(pixbuf->get_width(), pixbuf->get_height());
    int c = findGCD(a, b);
    int h = pixbuf->get_width() / c;
    int v = pixbuf->get_height() / c;
    if (h < 20) {  // give only usual numbers e.g. 16:9, 3:4 as 134:68 is not worth noting
        image += Glib::ustring::sprintf(", aspect %d:%d" , h, v);
    }
    appendList(chlds, "Width, Height", image);

    Glib::ustring chan(pixbuf->getPixbuf()->get_has_alpha() ? "yes" : "no");
    appendList(chlds,"Alpha", chan);	// at least this shoud relate to the image

	// options might be interesting ...
	std::map<Glib::ustring, Glib::ustring> map = pixbuf->getOptions();
	for (auto m : map) {
		if ("tEXt::Raw profile type exif" == m.first) {
			//std::cout << "ExifReader exifReader" << std::endl;
			ExifReader exifReader(*this);
			exifReader.decode(m.second);
		}
		else {
			//std::cout << "opt " << m.first << " = " << m.second << std::endl;
			Glib::ustring key(m.first);
			if (key.length() >= 6
			 &&	key.substr(0, 6) == "tEXt::") {
				key = key.substr(6);
			}
			appendList(chlds, key.c_str(), m.second);
		}
	}
}


// move to StringUtils, unify with Mon ?
Glib::ustring
ImageList::formatScale(guint64 value)   // , const char *suffix, double scaleFactor
{
//    std::string scale;
//    double mscale = scaleFactor * scaleFactor;
//    double gscale = mscale * scaleFactor;
//    if (value >= gscale) {
//        value /= gscale;
//        scale = "G";
//    }
//    else if (value >= mscale) {
//        value /= mscale;
//        scale = "M";
//    }
//    else if (value >= scaleFactor) {
//        value /= scaleFactor;
//        scale = "k";
//    }
//    else if (value < 10.0 / scaleFactor) {
//        value *= scaleFactor;
//        scale = "m";
//    }
//    else if (value < 10.0 / mscale) {
//        value *= mscale;
//        scale = "µ";
//    }
//    else if (value < 10.0 / gscale) {
//        value *= gscale;
//        scale = "n";
//    }
//    guint places = 0;
//    if (value < 1.0) {
//        places = 3;
//    }
//    else if (value < 10.0) {
//        places = 2;
//    }
//    else if (value < 100.0) {
//        places = 1;
//    }
//    //std::ostringstream oss1;
//    //oss1.precision(places);
//    //oss1 << std::fixed << value;
//    //oss1 << scale;
//    //oss1 << suffix;
//    //std::string max = oss1.str();
//    Glib::ustring max = Glib::ustring::sprintf("%.*f%s%s", places, value, scale, suffix);
//    return max;
    return Glib::format_size( (guint64)value,  Glib::FormatSizeFlags::FORMAT_SIZE_DEFAULT);   // FORMAT_SIZE_IEC_UNITS based on 1024
}

void
ImageList::setValues(Gtk::TreeIter& i, const char *name, const Glib::ustring& value)
{
    auto row = *i;
    Glib::ustring cName(name);
    row.set_value(m_variableColumns.m_name, cName);
    row.set_value(m_variableColumns.m_value, value);
}

Gtk::TreeIter
ImageList::appendList(const char *name, const Glib::ustring& value)
{
    auto i = append();
	setValues(i, name, value);
	return i;
}

Gtk::TreeIter
ImageList::appendList(Gtk::TreeIter& node, const char *name, const Glib::ustring& value)
{
	auto i = append((*node).children());
	setValues(i, name, value);
	return i;
}


void
ImageList::appendList(Gtk::TreeIter& node, const char *name, const Glib::DateTime& modified)
{
    auto i = append((*node).children());
    Glib::ustring vModified(modified.format("%x %X"));
	setValues(i, name, vModified);
}

Glib::RefPtr<ImageList>
ImageList::create()
{
    return Glib::RefPtr<ImageList>(new ImageList());
}