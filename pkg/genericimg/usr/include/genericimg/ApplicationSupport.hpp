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

#include <list>
#include <gtkmm.h>

class KeyConfig;

class finally
{
    std::function<void(void)> functor;
public:
    finally(const std::function<void(void)> &functor)
    : functor(functor) {}
    ~finally()
    {
        functor();
    }
};

class ApplicationSupport  {
public:
    ApplicationSupport(const std::shared_ptr<KeyConfig>& keyConfig);
    // create a nested support (for a child window)
    ApplicationSupport(ApplicationSupport* parent);
    virtual ~ApplicationSupport();

    void addWindow(Gtk::Window *window, const Glib::ustring& confKeyPref, int width, int height, const char* confGrp = nullptr);
    void removeWindow(Gtk::Window *window, const Glib::ustring& confKeyPref, const char* confGrp = nullptr);
    void setApplication(Gtk::Application* application);
    Gtk::Application* getApplication();
    void saveConfig();
    std::shared_ptr<KeyConfig> getConfig();

    void showError(const std::string& msg, Gtk::MessageType type = Gtk::MESSAGE_ERROR, Gtk::ButtonsType buttons = Gtk::BUTTONS_CANCEL);
    bool askYesNo(const std::string& msg, Gtk::MessageType type = Gtk::MESSAGE_QUESTION, Gtk::ButtonsType buttons = Gtk::BUTTONS_YES_NO);
    void addDialogYesNo(Gtk::FileChooserDialog& dlg);
protected:
    Gtk::Window* getTopWindow();

private:
    ApplicationSupport* m_parent{nullptr};
    std::shared_ptr<KeyConfig> m_config;
    Gtk::Application* m_application{nullptr};
    Gtk::Window* m_window{nullptr};
    static constexpr auto CONF_POSX = "PosX";
    static constexpr auto CONF_POSY = "PosY";
    static constexpr auto CONF_WIDTH = "Width";
    static constexpr auto CONF_HEIGHT = "Height";

};

