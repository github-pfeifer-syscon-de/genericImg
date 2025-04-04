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

#include "BinView.hpp"
#include "ImageView.hpp"
#include "ImageOptionDialog.hpp"
#include "DisplayImage.hpp"
#include "KeyConfig.hpp"


ImageFilter::ImageFilter(Gdk::PixbufFormat &format)
: Gtk::FileFilter()
, m_format{format}
{
}

Glib::RefPtr<ImageFilter>
ImageFilter::create(Gdk::PixbufFormat &format)
{
    return Glib::RefPtr<ImageFilter>(new ImageFilter(format));
}

Gdk::PixbufFormat
ImageFilter::getFormat()
{
    return m_format;
}

void
ImageFilter::addFormats(Gtk::FileChooserDialog &dialog, Glib::ustring &prefFormat)
{
    for (auto format : Gdk::Pixbuf::get_formats()) {
        if (format.is_writable()) {
            auto filter_image = ImageFilter::create(format);
            filter_image->set_name(format.get_description());
            for (Glib::ustring mime : format.get_mime_types()) {
                filter_image->add_mime_type(mime);
            }
            for (Glib::ustring ext : format.get_extensions()) {
                Glib::ustring pattern("*.");
                pattern += ext;
                filter_image->add_pattern(pattern);
            }
            dialog.add_filter(filter_image);
            if (format.get_name() == prefFormat) {
                dialog.set_filter(filter_image);
            }
        }
    }
}

template<class T,typename G>
ImageView<T,G>::ImageView(
    G* cobject
    , const Glib::RefPtr<Gtk::Builder>& builder
    , std::shared_ptr<Mode> mode
    , ApplicationSupport& appSupport
    , bool instantiateImageview )
: T(cobject)
, m_mode{mode}
, m_appSupport{&appSupport}
{
    m_appSupport.addWindow(this, CONF_PREFIX, 640, 480, CONF_GROUP);

    auto scrollObj = builder->get_object("imageScroll");
    m_scroll = Glib::RefPtr<Gtk::ScrolledWindow>::cast_dynamic(scrollObj);
    auto panedObj = builder->get_object("paned");
    m_paned = Glib::RefPtr<Gtk::Paned>::cast_dynamic(panedObj);
    builder->get_widget_derived("binDraw", m_binView);
    if (instantiateImageview) {
        builder->get_widget_derived("imageDraw", m_content, m_appSupport, this);
    }
    auto tableObj = builder->get_object("viewTable");
    m_table = Glib::RefPtr<Gtk::TreeView>::cast_dynamic(tableObj);
    m_listStore = ImageList::create();
    m_table->append_column("Name", ImageList::m_variableColumns.m_name);
    m_table->append_column("Value", ImageList::m_variableColumns.m_value);
    m_table->set_model(m_listStore);

    auto config = m_appSupport.getConfig();
    auto fitObj = builder->get_object("fit");
    m_fitRadio = Glib::RefPtr<Gtk::RadioButton>::cast_dynamic(fitObj);
    m_fitRadio->set_sensitive(true);
    auto orgObj = builder->get_object("nativ");
    m_nativRadio = Glib::RefPtr<Gtk::RadioButton>::cast_dynamic(orgObj);
    m_nativRadio->set_sensitive(true);
    Gtk::RadioButtonGroup viewGroup;
    m_nativRadio->set_group(viewGroup);
    m_fitRadio->set_group(viewGroup);
    if (config->hasKey(CONF_GROUP, CONF_PANED)) {
        gint iDivPos = config->getInteger(CONF_GROUP, CONF_PANED);
        m_paned->set_position(iDivPos);
    }

    // will be fired for actived & deactivated ... so one handler will suffice. Have to check if n > 2
    m_nativRadio->signal_toggled().connect(
        sigc::bind(sigc::mem_fun(*this, &ImageView<T,G>::on_menu_view), ViewMode::NATIVE));
    //fitRadio->signal_clicked().connect(
    //    sigc::bind(sigc::mem_fun(*this, &ImageView<T,G>::on_menu_view), ViewMode::FIT));

    builder->get_widget("prev", m_prevBtn);
    m_prevBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &ImageView<T,G>::on_menu_prev));
    m_prevBtn->set_sensitive(mode->hasNavigation());
    builder->get_widget("next", m_nextBtn);
    m_nextBtn->signal_clicked().connect(
        sigc::mem_fun(*this, &ImageView<T,G>::on_menu_next));
    m_nextBtn->set_sensitive(mode->hasNavigation());

    //show_all_children();
    T::add_events(Gdk::EventMask::BUTTON_PRESS_MASK
             | Gdk::EventMask::BUTTON_RELEASE_MASK
             | Gdk::EventMask::SCROLL_MASK
             | Gdk::EventMask::BUTTON_MOTION_MASK
             | Gdk::EventMask::POINTER_MOTION_MASK);

    T::signal_show().connect(      // delay until fully initialized
        [this,config] () {
            ViewMode viewMode;
            if (config->hasKey(CONF_GROUP, CONF_VIEW)) {
                gint iViewMode = config->getInteger(CONF_GROUP, CONF_VIEW);
                viewMode = (ViewMode)iViewMode;
                m_content->setViewMode(viewMode);
            }
            else {
                viewMode = m_content->getViewMode();
            }
            switch (viewMode) {
            case ViewMode::NATIVE:
                m_nativRadio->set_active(true);
                break;
            case ViewMode::FIT:
                m_fitRadio->set_active(true);
                break;
            }

            showFront();
        });
}

template<class T, typename G>
void
ImageView<T,G>::showView(int32_t front, std::vector<Glib::RefPtr<Gio::File>>& picts, ApplicationSupport& m_appSupport)
{
    auto builder = Gtk::Builder::create();
    try {
        Gtk::Application* appl = m_appSupport.getApplication();
        builder->add_from_resource(appl->get_resource_base_path() + "/view.ui");
        ImageView<Gtk::Window,GtkWindow>* viewWin;
        auto mode = std::make_shared<PagingMode>(front, picts);
        builder->get_widget_derived("ImageView", viewWin, mode, m_appSupport);
        viewWin->show();
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load view.ui: " << ex.what() << std::endl;
    }
}

template<class T, typename G>
Glib::RefPtr<Gio::File>
ImageView<T,G>::getDefaultDir()
{
    auto config = m_appSupport.getConfig();
    Glib::RefPtr<Gio::File> f;
    if (config->hasKey(CONF_GROUP_MAIN, CONF_PATH)) {
        Glib::ustring path = config->getString(CONF_GROUP_MAIN, CONF_PATH);
        f = Gio::File::create_for_path(path);
    }
    else {
        bool retry = false;
        do {
            Gtk::FileChooserDialog fc(*this
                        , "Pleas provide your default image location"
                        , Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER
                        , Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT);
            fc.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
            fc.add_button("_Open", Gtk::RESPONSE_ACCEPT);
            if (fc.run() == Gtk::RESPONSE_ACCEPT) {
                f = fc.get_file();
                retry = !f->query_exists();
            }
            else {
                return f;
            }
        } while (retry);
        config->setString(CONF_GROUP_MAIN, CONF_PATH, f->get_path());
    }
    return f;
}

template<class T, typename G>
std::shared_ptr<Mode>
ImageView<T,G>::createDirMode()
{
    auto f = getDefaultDir();
    if (!f) {
        return nullptr;
    }
    // collect all available content types
    auto fmts = Gdk::Pixbuf::get_formats();
    std::map<Glib::ustring, Gdk::PixbufFormat> mimeTypes;
    for (auto fmt : fmts) {
        for (auto mime : fmt.get_mime_types()) {
            mimeTypes.insert(std::make_pair(mime, fmt));
        }
    }
    auto en = f->enumerate_children("standard::*", Gio::FileQueryInfoFlags::FILE_QUERY_INFO_NONE);
    // prefere ustring as the sorting is more consistent
    //   using collate_key would be more effective ...
    //     the docs suggest using compare ...?
    std::map<Glib::ustring, Glib::RefPtr<Gio::File>> map;
    while (true) {
        auto fi = en->next_file();
        if (!fi) {
            break;
        }
        if (fi->get_file_type() == Gio::FileType::FILE_TYPE_REGULAR) {  // enum should follow symlinks, so regular should catch linked files as well
            auto cnt = fi->get_content_type();
            //std::cout << "Found file " << fi->get_name() << " type " << cnt << std::endl;
            if (mimeTypes.find(cnt) != mimeTypes.end()) {
                auto fpath = Glib::canonicalize_filename(fi->get_name(), f->get_path());
                auto file = Gio::File::create_for_path(fpath);
                auto displayName = Glib::ustring(fi->get_display_name());
                map.insert(std::make_pair(displayName, file));
            }
        }
    }
    // map has sort names
    std::vector<Glib::RefPtr<Gio::File>> fs;
    for (auto entry : map) {
        fs.push_back(std::move(entry.second));
    }
    return std::make_shared<PagingMode>(0, fs);
}

template<class T, typename G>
void
ImageView<T,G>::setFile(const Glib::RefPtr<Gio::File>& file)
{
    T::set_title(file->get_basename());
    m_content->setFile(file);
    m_listStore->fillList(file);
}

template<class T, typename G>
void
ImageView<T,G>::showFront()
{
    if (!m_mode->isComplete()) {
        auto mode = createDirMode();
        if (mode) {
            m_mode = mode;
        }
        else {
            m_prevBtn->set_sensitive(false);
            m_nextBtn->set_sensitive(false);
            return;
        }
    }
    m_mode->show(this);
}

template<class T, typename G>
void
ImageView<T,G>::setDisplayImage(Glib::RefPtr<DisplayImage>& displayImage)
{
    T::set_title("Edit");
    m_listStore->clear();
    m_content->setPixbuf(displayImage);
}

template<class T, typename G>
void
ImageView<T,G>::updateImageInfos(Glib::RefPtr<DisplayImage>& pixbuf)
{
	m_binView->setPixbuf(pixbuf);
	m_listStore->fillList(pixbuf);
	m_table->expand_all();
}

template<class T, typename G>
void
ImageView<T,G>::clearUpdateImageInfos(Glib::RefPtr<DisplayImage>& pixbuf)
{
    m_listStore->clear();
    updateImageInfos(pixbuf);
}

template<class T, typename G>
void
ImageView<T,G>::on_hide()
{
    auto config = m_appSupport.getConfig();
    gint iDivPos = m_paned->get_position();
    config->setInteger(CONF_GROUP, CONF_PANED, iDivPos);   // will be saved with window
    m_appSupport.saveConfig();

    m_appSupport.removeWindow(this, CONF_PREFIX, CONF_GROUP);

    Gtk::Window::on_hide();
    //delete this;    // this might not be the nicest way, but it works
}

template<class T, typename G>
bool
ImageView<T,G>::on_motion_notify_event(GdkEventMotion* event)
{
    //std::cout << "motion state 0x" <<  std::hex << event->state
    //          << "btn1 0x" << Gdk::ModifierType::BUTTON1_MASK
    //          << std::endl << std::dec;

	bool drag = event->state & Gdk::ModifierType::BUTTON1_MASK;
    if (m_select) {
		auto cursor = m_content->mouse_pressed(event->x, event->y, drag);
		if (!cursor) {
			cursor = Gdk::Cursor::create(T::get_display(), Gdk::ARROW);
		}
		T::get_window()->set_cursor(cursor);
        return true;
    }
	else if (drag) {
        double dx = m_dragStartX - event->x;
        double dy = m_dragStartY - event->y;
        //std::cout << "motion dx " << dx << " dy " << dy << std::endl;

        Glib::RefPtr<Gtk::Adjustment> hadjust = m_scroll->get_hadjustment();
        hadjust->set_value(hadjust->get_value() + dx);
        Glib::RefPtr<Gtk::Adjustment> vadjust = m_scroll->get_vadjustment();
        vadjust->set_value(vadjust->get_value() + dy);

        m_dragStartX = event->x;
        m_dragStartY = event->y;
        return true;
    }
    return false;
}

template<class T, typename G>
bool
ImageView<T,G>::on_scroll_event(GdkEventScroll* scroll_event)
{
    // may work for devices supporting scroll (tablet?)
    //std::cout << "scroll dx " << scroll_event->delta_x << " dy " << scroll_event->delta_y << std::endl;

    Glib::RefPtr<Gtk::Adjustment> hadjust = m_scroll->get_hadjustment();
    hadjust->set_value(hadjust->get_value() + scroll_event->delta_x);
    Glib::RefPtr<Gtk::Adjustment> vadjust = m_scroll->get_vadjustment();
    vadjust->set_value(vadjust->get_value() + scroll_event->delta_y);
    //Gtk::Viewport* view = dynamic_cast<Gtk::Viewport *>(m_content->get_parent());
    //if (view) {
    //    view->
    //}
    return true;
    //return false;
}

template<class T, typename G>
bool
ImageView<T,G>::on_button_press_event(GdkEventButton* event)
{
    if (event->button == GDK_BUTTON_PRIMARY) {
        m_dragStartX = event->x;
        m_dragStartY = event->y;
    }
    if (gdk_event_triggers_context_menu((GdkEvent*)event))  {       // event->button == GDK_BUTTON_SECONDARY
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        Gtk::Menu* popupMenu = build_popup(x, y);
        popupMenu->show_all();
        // deactivate prevent item signals to get generated ...
        // signal_unrealize will never get generated
        popupMenu->attach_to_widget(*this);     // this does the trick and calls the destructor
        popupMenu->popup(event->button, event->time);

        return true; // It has been handled.
    }
    return false;
}

template<class T, typename G>
Gtk::Menu *
ImageView<T,G>::build_popup(int x, int y)
{
    //std::cout << "ImageView<T,G>::build_popup" << std::endl;
    // managed works when used with attach ...
    auto pMenuPopup = Gtk::make_managed<Gtk::Menu>();
    if (m_content->getDisplayImage()) {
        auto save = Gtk::make_managed<Gtk::MenuItem>("_Save", true);
        save->signal_activate().connect(sigc::mem_fun(*this, &ImageView<T,G>::on_menu_save));
        pMenuPopup->append(*save);
    }
    if (m_mode->hasNavigation()) {
        auto last = Gtk::make_managed<Gtk::MenuItem>("_View", true);
        pMenuPopup->append(*last);
        auto subMenu = Gtk::make_managed<Gtk::Menu>();
        last->set_submenu(*subMenu);
        m_mode->buildMenu(subMenu, this, &ViewIntf::on_menu_n);
    }
    auto select = Gtk::make_managed<Gtk::CheckMenuItem>("_Select", true);
    select->set_active(m_select);
    select->signal_activate().connect(sigc::mem_fun(*this, &ImageView<T,G>::on_select));
    pMenuPopup->append(*select);

    return pMenuPopup;
}

template<class T, typename G>
void
ImageView<T,G>::on_select()
{
    m_select = !m_select;
	if (!m_select) {
		if (m_appSupport.askYesNo("Crop Image?")) {
			m_content->crop();
		}
	}
    m_content->setSelected(m_select);
}

template<class T, typename G>
void
ImageView<T,G>::on_menu_save()
{
    Gtk::FileChooserDialog dialog("Save file"
								, Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    m_appSupport.addDialogYesNo(dialog);

    auto config = m_appSupport.getConfig();
    if (config->hasKey(CONF_GROUP, CONF_PATH)) {
        Glib::ustring path = config->getString(CONF_GROUP, CONF_PATH);
        Glib::RefPtr<Gio::File> fPath = Gio::File::create_for_path(path);
        Glib::RefPtr<Gio::File> fParent = fPath->get_parent();     // as we stored selected file
        if (fParent->query_exists()) {
            dialog.set_current_folder(fParent->get_path());
        }
    }

    //Add filters, so that only certain file types can be selected:
    Glib::ustring prefFormat = config->getString(CONF_GROUP, CONF_FILTER, "jpeg");
    ImageFilter::addFormats(dialog, prefFormat);
    int result = dialog.run();
	dialog.hide();	// only one open dialog at a time (prevent hiding newer instance)
    if (result == Gtk::RESPONSE_OK) {
        Glib::RefPtr<Gtk::FileFilter> fileFilter = dialog.get_filter();
        Glib::RefPtr<ImageFilter> filter = Glib::RefPtr<ImageFilter>::cast_dynamic(fileFilter);
        if (filter) {
            Gdk::PixbufFormat format = filter->getFormat();
            save_image(dialog.get_filename(), format);
        }
        else {
            m_appSupport.showError("Coud not identify format !!!");
        }
    }
}

template<class T, typename G>
void
ImageView<T,G>::save_image(Glib::ustring filename, Gdk::PixbufFormat& format)
{
    //cout << "on save " << filename << " format " << filter->get_name() << endl;
    auto config = m_appSupport.getConfig();
    config->setString(CONF_GROUP, CONF_PATH, filename);

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
	std::vector<Glib::ustring> keys;
	std::vector<Glib::ustring> opts;
	auto existOpts = m_content->getDisplayImage()->getOptions();
	if (ImageOptionDialog::hasOptions(format, file, existOpts)) {
		ImageOptionDialog optDlg(*this, format, file, existOpts);
		if (optDlg.run() != Gtk::RESPONSE_OK) {
			return;
		}
		optDlg.addOptions(keys, opts);
	}
    try {
        config->setString(CONF_GROUP, CONF_FILTER, format.get_name());
        m_content->getDisplayImage()->getPixbuf()->save(filename, format.get_name(), keys, opts);
    }
    catch (const Glib::Error &ex) {
        m_appSupport.showError(ex.what());
    }
}

template<class T, typename G>
void
ImageView<T,G>::on_menu_next()
{
    m_mode->next();
    showFront();
}

template<class T, typename G>
void
ImageView<T,G>::on_menu_prev()
{
    m_mode->prev();
    showFront();
}

template<class T, typename G>
void
ImageView<T,G>::refresh()
{
    m_mode->show(this);
}

template<class T, typename G>
void
ImageView<T,G>::on_menu_n(int32_t n)
{
    m_mode->set(n);
    showFront();
}

template<class T, typename G>
void
ImageView<T,G>::on_menu_view(ViewMode viewMode)
{
    // as the signal itself does not give actual activation
    viewMode = m_fitRadio->get_active() ? ViewMode::FIT : ViewMode::NATIVE;
    //std::cout << "ImageView<T,G>::on_menu_view" << (int)viewMode << std::endl;
    m_content->setViewMode(viewMode);
    auto config = m_appSupport.getConfig();
    auto iViewMode = static_cast<gint>(viewMode);
    config->setInteger(CONF_GROUP, CONF_VIEW, iViewMode);
}

template<class T, typename G>
bool
ImageView<T,G>::is_mime_gdk_readable(const Glib::ustring& mime)
{
	std::vector<Gdk::PixbufFormat> formats = Gdk::Pixbuf::get_formats();
	bool supported = false;
	for (Gdk::PixbufFormat format : formats) {	// only include images that are displayable (readable)
		for (auto availMime : format.get_mime_types()) {
			if (mime == availMime
				&& !format.is_disabled()) {
				supported = true;
				break;
			}
		}
	}
	return supported;
}

template<class T, typename G>
std::shared_ptr<Mode>
ImageView<T,G>::getMode()
{
    return m_mode;
}