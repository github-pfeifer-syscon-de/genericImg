/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 rpf
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
#include <png.h>
#include <glibmm.h>
#include <stdio.h>

#include "ImageUtils.hpp"

bool
ImageUtils::grayscalePng(Glib::RefPtr<Gdk::Pixbuf>& pixbuf, const Glib::ustring& filename)
{
   // the default Gdk/Cairo? will only allow writing color png...
    //   presumes r = g = b as we created it that way
    bool ret = false;
    int32_t width = pixbuf->get_width();
    int32_t height = pixbuf->get_height();
    int32_t sample_depth = 8;
    auto graydata = new uint8_t[width*height];
    auto rowptr = new uint8_t*[height];
    if (graydata && rowptr) {
        //   create packed grayscale buffer
        auto bytePerPixel = pixbuf->get_n_channels();
        //std::cout << "width " << width
        //          << " height " << height
        //          << " bytePerPixel " << bytePerPixel
        //          << std::endl;
        for (int32_t y = 0; y < height; ++y) {
            //std::cout << "Convertring row " << y << std::endl;
            // use add 2 to use green
            auto rows = pixbuf->get_pixels() + 2 + (y * pixbuf->get_rowstride());
            auto rowd = &graydata[y * width];
            rowptr[y] = rowd;       // fill pointer for rows
            for (int32_t x = 0; x < width; ++x) {
                *rowd = *rows;
                ++rowd;
                rows += bytePerPixel;
            }
        }
        auto row_pointers = reinterpret_cast<png_bytepp>(rowptr);
        int interlace_type = PNG_INTERLACE_NONE; // ? PNG_INTERLACE_ADAM7
        int color_type = PNG_COLOR_TYPE_GRAY;
        FILE *fp = fopen(filename.c_str(), "wb");
        if (fp) {
            png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            png_init_io(png_ptr, fp);
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_set_IHDR(png_ptr, info_ptr
                    , width, height
                    , sample_depth
                    , color_type, interlace_type
                    , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            ret = true;
        }
        else {
            std::cout << "Cannot export " << filename << std::endl;
        }
    }
    else {
        std::cout << "Not enough memory failed to create " << filename << std::endl;
    }
    if (rowptr) {
        delete[] rowptr;
    }
    if (graydata) {
        delete[] graydata;
    }
    return ret;
}

bool
ImageUtils::blackandwhitePng(Glib::RefPtr<Gdk::Pixbuf>& pixbuf, const Glib::ustring& filename)
{
    //   presumes r = g = b as we created it that way
    bool ret = false;
    uint32_t width = static_cast<uint32_t>(pixbuf->get_width());
    int32_t height = pixbuf->get_height();
    int32_t sample_depth = 1;
    int32_t bytesStride = (width + 7) / 8;
    auto graydata = new uint8_t[bytesStride*height];
    auto rowptr = new uint8_t*[height];
    if (graydata && rowptr) {
        //   create packed grayscale buffer
        auto bytePerPixel = pixbuf->get_n_channels();
        std::cout << "width " << width
                  << " height " << height
                  << " (src)bytePerPixel " << bytePerPixel
                  << " (dest)byteStride " << bytesStride
                  << std::endl;
        for (int32_t y = 0; y < height; ++y) {
            //std::cout << "Convertring row " << y << std::endl;
            // use add 2 to use green
            auto rows = pixbuf->get_pixels() + 2 + (y * pixbuf->get_rowstride());
            auto rowd = &graydata[y * bytesStride];
            rowptr[y] = rowd;       // fill pointer for rows
            uint8_t byted = 0;
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t mask = 0x80u >> (x & 0x7u);
                byted |= *rows > 0x7fu ? mask : 0u;
                if (mask == 0x01u || x == (width-1)) {
                    *rowd = byted;
                    ++rowd;
                    byted = 0;
                }
                rows += bytePerPixel;
            }
        }
        auto row_pointers = reinterpret_cast<png_bytepp>(rowptr);
        int interlace_type = PNG_INTERLACE_NONE; // ? PNG_INTERLACE_ADAM7
        int color_type = PNG_COLOR_TYPE_GRAY;
        FILE *fp = fopen(filename.c_str(), "wb");
        if (fp) {
            png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            png_init_io(png_ptr, fp);
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_set_IHDR(png_ptr, info_ptr
                    , width, height
                    , sample_depth
                    , color_type, interlace_type
                    , PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            ret = true;
        }
        else {
            std::cout << "Cannot export " << filename << std::endl;
        }
    }
    else {
        std::cout << "Not enough memory failed to create " << filename << std::endl;
    }
    if (rowptr) {
        delete[] rowptr;
    }
    if (graydata) {
        delete[] graydata;
    }
    return ret;
}