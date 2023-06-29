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

#include "ApplicationSupport.hpp"

ApplicationSupport::ApplicationSupport(Glib::ustring configName)
: m_parent{nullptr}
, m_configName{configName}
, m_config{nullptr}
, m_application{nullptr}
, m_window{nullptr}
{
}

ApplicationSupport::ApplicationSupport(ApplicationSupport* parent)
: m_parent(parent)
, m_configName(parent->m_configName)
, m_config(parent->m_config)
, m_application(parent->m_application)
, m_window(nullptr)
{
}


ApplicationSupport::~ApplicationSupport()
{
    if (m_parent == nullptr
     && m_config) {
        delete m_config;
        m_config = nullptr;
    }
}

void
ApplicationSupport::addWindow(Gtk::Window *window, const Glib::ustring confKeyPref, int width, int height, const char* confGrp)
{
    if (m_parent != nullptr) {
        Gtk::Window* parentWindow = getTopWindow();
        if (parentWindow) {
            window->set_transient_for(*parentWindow);     // auto chain
        }
    }
    m_window = window;

    auto config = getConfig();
    if (confGrp) {
        if (config->has_group(confGrp)
         && config->has_key(confGrp, confKeyPref + CONF_POSX)) {
            int width = config->get_integer(confGrp, confKeyPref + CONF_WIDTH);
            int height = config->get_integer(confGrp, confKeyPref + CONF_HEIGHT);
            if (width > 0
             && height > 0) {
                window->set_default_size(width, height);
            }
            int posx = config->get_integer(confGrp, confKeyPref + CONF_POSX);
            int posy = config->get_integer(confGrp, confKeyPref + CONF_POSY);
            if (posx > 0
             && posy > 0) {
                window->move(posx, posy);
            }
            //std::cout << "loadWinConf " << confGrp
            //          << " width " << width
            //          << " height " << height
            //          << " posx " << posx
            //          << " posy " << posy << std::endl;
        }
        else {
            window->set_default_size(width, height);
        }
    }


}

void
ApplicationSupport::removeWindow(Gtk::Window *window, const Glib::ustring confKeyPref, const char* confGrp)
{
    if (m_window != nullptr
     && m_window == window) {
        m_window = nullptr;

        if (confGrp) {
            int width;
            int height;
            window->get_size(width, height);
            int posx;
            int posy;
            window->get_position(posx, posy);
            auto config = getConfig();
            //std::cout << "saveWinConf " << confGrp
            //          << " pref " << confKeyPref
            //          << " width " << width
            //          << " height " << height
            //          << " posx " << posx
            //          << " posy " << posy << std::endl;
            config->set_integer(confGrp, confKeyPref + CONF_WIDTH, width);
            config->set_integer(confGrp, confKeyPref + CONF_HEIGHT, height);
            config->set_integer(confGrp, confKeyPref + CONF_POSX, posx);
            config->set_integer(confGrp, confKeyPref + CONF_POSY, posy);
            saveConfig();
        }
    }
    else {
        std::cerr << "Try to remove non matching window"
                  << " expecting " << (m_window != nullptr ? m_window->get_title() : "null")
                  << " got " << (window != nullptr ? window->get_title() : "null")
                  << std::endl;
    }
}

std::string
ApplicationSupport::getConfigName()
{
    std::string fullPath = Glib::canonicalize_filename(m_configName.c_str(), Glib::get_user_config_dir());
    return fullPath;
}

void
ApplicationSupport::saveConfig()
{
    if (m_config) {
        m_config->save_to_file(getConfigName());
    }
}

Glib::KeyFile*
ApplicationSupport::getConfig()
{
    if (!m_config) {
        m_config = new Glib::KeyFile();
        try {
            m_config->load_from_file(getConfigName());
        }
        catch (const Glib::FileError& exc) {
            std::cerr << "Cound not read " << exc.what() << " config " << m_configName << " (it may not yet exist and will be created)." << std::endl;
        }
    }
    return m_config;
}

void
ApplicationSupport::showError(const std::string& msg, Gtk::MessageType type, Gtk::ButtonsType buttons)
{
    Gtk::Window* window = getTopWindow();
    if (window) {
        Gtk::MessageDialog msgDlg(*window, msg, false, type, buttons, true);
        msgDlg.run();
    }
    else {
        Gtk::MessageDialog msgDlg(msg, false, type, buttons, true);
        msgDlg.run();
    }
}

Gtk::Window*
ApplicationSupport::getTopWindow()
{
    if (m_window != nullptr) {
        return m_window;
    }
    if (m_parent != nullptr) {
        return m_parent->getTopWindow();
    }
    return nullptr;
}

bool
ApplicationSupport::askYesNo(const std::string& msg, Gtk::MessageType type, Gtk::ButtonsType buttons)
{
    Gtk::Window* window = getTopWindow();
    int ret;
    if (window) {
        Gtk::MessageDialog msgDlg(*window, msg, false, type, buttons, true);
        ret = msgDlg.run();
    }
    else {
        Gtk::MessageDialog msgDlg(msg, false, type, buttons, true);
        ret = msgDlg.run();
    }
    return ret == Gtk::RESPONSE_YES;
}

void
ApplicationSupport::addDialogYesNo(Gtk::FileChooserDialog& dialog)
{
    std::string okName = "_Open";
    if (dialog.get_action() == Gtk::FILE_CHOOSER_ACTION_SAVE) {
        okName = "_Save";
    }
    else if (dialog.get_action() == Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER) {
        okName = "_Create";
    }
    //Add response buttons to the dialog:
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button(okName, Gtk::RESPONSE_OK);

    Gtk::Window* window = getTopWindow();
    if (window) {
        dialog.set_transient_for(*window);
    }
}

void
ApplicationSupport::setApplication(Gtk::Application* application)
{
    m_application = application;
}

Gtk::Application*
ApplicationSupport::getApplication()
{
    return m_application;
}
