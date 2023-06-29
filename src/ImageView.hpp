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

#include <vector>
#include <memory>
#include <gtkmm.h>

#include "ImageArea.hpp"
#include "ApplicationSupport.hpp"
#include "ImageList.hpp"
#include "Mode.hpp"

class ImageFilter
: public Gtk::FileFilter
{
public:
    ImageFilter(Gdk::PixbufFormat &format);
    virtual ~ImageFilter() = default;
    Gdk::PixbufFormat getFormat();

    static void addFormats(Gtk::FileChooserDialog &dialog, Glib::ustring &prefered);
    static Glib::RefPtr<ImageFilter> create(Gdk::PixbufFormat &format);

private:
    Gdk::PixbufFormat m_format;
};

class BinView;
class DisplayImage;

class ImageView
: public Gtk::ApplicationWindow
, public ViewIntf
{
public:
    ImageView(BaseObjectType* cobject
            , const Glib::RefPtr<Gtk::Builder>& builder
            , std::shared_ptr<Mode> mode
            , ApplicationSupport& appSupport
            , bool instantiateImageview = true);
    virtual ~ImageView() = default;

    virtual void on_hide() override;
    void showFront();
    void updateImageInfos(Glib::RefPtr<DisplayImage>& pixbuf);
    void setFile(const Glib::RefPtr<Gio::File>& file) override;
    void setDisplayImage(Glib::RefPtr<DisplayImage>& displayImage);

    static void showView(int32_t front, std::vector<Glib::RefPtr<Gio::File>>& picts, ApplicationSupport& m_appSupport);
    static bool is_mime_gdk_readable(const Glib::ustring& mime);
    std::shared_ptr<Mode> getMode();
    void refresh();
protected:
    Glib::RefPtr<Gio::File> getDefaultDir();
    std::shared_ptr<Mode> createDirMode();
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_scroll_event(GdkEventScroll* scroll_event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;
    void on_menu_save();
    void save_image(Glib::ustring filename, Gdk::PixbufFormat& format);
    void on_menu_next();
    void on_menu_prev();
    void on_menu_n(gint n) override;
    void on_menu_view(ViewMode viewMode);
    void on_select();
    virtual Gtk::Menu* build_popup();

    ImageArea* m_content{nullptr};
    std::shared_ptr<Mode> m_mode;
    ApplicationSupport m_appSupport;
    Glib::RefPtr<Gtk::ScrolledWindow> m_scroll;
    Glib::RefPtr<Gtk::Paned> m_paned;
    Glib::RefPtr<Gtk::TreeView> m_table;
    Glib::RefPtr<ImageList> m_listStore;
    double m_dragStartX{0.0},m_dragStartY{0.0};
    Glib::RefPtr<Gtk::RadioButton> m_fitRadio;
    Glib::RefPtr<Gtk::RadioButton> m_nativRadio;
    BinView* m_binView{nullptr};
    Gtk::Button* m_prevBtn{nullptr};
    Gtk::Button* m_nextBtn{nullptr};
    bool m_select{false};
    static constexpr auto CONF_GROUP{"view"};
    static constexpr auto CONF_PREFIX{"view"};
    static constexpr auto CONF_PATH{"path"};
    static constexpr auto CONF_FILTER{"filter"};
    static constexpr auto CONF_VIEW{"view"};
    static constexpr auto CONF_PANED{"paned"};
    static constexpr auto CONF_GROUP_MAIN{"main"};
};

