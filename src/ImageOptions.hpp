/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <map>
#include <memory>



class ImageOption {
public :
    virtual ~ImageOption() = default;
    virtual Glib::ustring getValue() = 0;
    virtual void setValue(Glib::ustring& val) = 0;
    virtual const Glib::ustring getName() = 0;
    virtual const Glib::ustring getOutputKey() = 0;
    void setExport(bool exp);
    bool isExport();
protected :
    ImageOption(bool exp);

    virtual int getMax() = 0;
private:
    bool m_export;
};


class ImageOptions {
public :
    virtual ~ImageOptions();
    static std::shared_ptr<ImageOptions> getOptions(Gdk::PixbufFormat& format, std::map<Glib::ustring, Glib::ustring>& opts);
    const std::map<Glib::ustring, std::shared_ptr<ImageOption>>& getOptions();
    void add(std::map<Glib::ustring, Glib::ustring>& opts);
protected :
    ImageOptions();
    ImageOptions(const ImageOptions& other);

    virtual bool matches(Gdk::PixbufFormat& format) = 0;


    std::map<Glib::ustring, std::shared_ptr<ImageOption>> m_options;
};

class AdjustmentImageOption : public ImageOption {
public:
    AdjustmentImageOption(bool exp);
    virtual ~AdjustmentImageOption();
    Glib::RefPtr<Gtk::Adjustment> getAdjustment();
    virtual int getMin() = 0;
    virtual int getMax() = 0;
    virtual int getDefault() = 0;
    Glib::ustring getValue();
    void setValue(Glib::ustring& value);
protected:
    Glib::RefPtr<Gtk::Adjustment> m_adjustment;
private:

};

class QualityImageOption : public AdjustmentImageOption {
public:
    QualityImageOption();
    virtual ~QualityImageOption();
    int getMin() override;
    int getMax() override;
    int getDefault() override;
    const Glib::ustring getName() override;
    const Glib::ustring getOutputKey() override;
};


class JpegImageOptions : public ImageOptions {
public :
    JpegImageOptions();
    virtual ~JpegImageOptions();

    bool matches(Gdk::PixbufFormat& format) override;

};

class TextImageOption : public ImageOption {
public:
    TextImageOption(bool exp);
    virtual ~TextImageOption();

    Glib::RefPtr<Gtk::EntryBuffer> getBuffer();
    virtual int getMax() = 0;
    Glib::ustring getValue();
    void setValue(Glib::ustring& value);
protected:
    Glib::RefPtr<Gtk::EntryBuffer> m_buffer;
private:
};

class GenericImageOption : public TextImageOption {
public:
    GenericImageOption(const Glib::ustring& key, const Glib::ustring& outputKey, bool exp);
    virtual ~GenericImageOption();

    int getMax() override;
    const Glib::ustring getName() override;
    const Glib::ustring getOutputKey() override;
private:
    const Glib::ustring m_key;
    const Glib::ustring m_outputKey;
};


class PngImageOptions : public ImageOptions {
public :
    PngImageOptions();
    virtual ~PngImageOptions();

    bool matches(Gdk::PixbufFormat& format) override;

};

