/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf 
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


#include "Mode.hpp"

PagingMode::PagingMode(int32_t front, std::vector<Glib::RefPtr<Gio::File>>& picts)
: m_front{front}
, m_picts{picts}
{
}

Glib::RefPtr<Gio::File>
PagingMode::getFrontFile()
{
    if (m_front >= 0
     && static_cast<uint32_t>(m_front) < m_picts.size()) {
        return m_picts[m_front];
    }
    return Glib::RefPtr<Gio::File>();
}

void
PagingMode::show(ViewIntf* viewIntf)
{
    Glib::RefPtr<Gio::File> file = getFrontFile();
    if (file) {
        viewIntf->setFile(file);
    }
}

void
PagingMode::buildMenu(Gtk::Menu* subMenu, ViewIntf* imageView, function_ptr fun)
{
    int32_t n = 0;
    for (auto p : m_picts)  {
        Glib::ustring name = p->get_basename();
        auto lastN = Gtk::make_managed<Gtk::MenuItem>(name, true);
        lastN->signal_activate().connect(
            sigc::bind(sigc::mem_fun(*imageView, fun), n));
        subMenu->append(*lastN);
        ++n;
    }
}

void
PagingMode::next()
{
    ++m_front;
    m_front %= static_cast<int32_t>(m_picts.size());
}

void
PagingMode::prev()
{
    --m_front;
    if (m_front < 0) {
        m_front += static_cast<uint32_t>(m_picts.size());
    }
}

void
PagingMode::set(int32_t n)
{
    m_front = n;
    m_front %= static_cast<uint32_t>(m_picts.size());
}

bool
PagingMode::isComplete()
{
    return !m_picts.empty();
}

int32_t
PagingMode::get()
{
    return m_front;
}


std::vector<Glib::RefPtr<Gio::File>>
PagingMode::getPicts()
{
    return m_picts;
}

bool
PagingMode::join(std::shared_ptr<Mode> other)
{
    auto add = std::dynamic_pointer_cast<PagingMode>(other);
    if (add) {
        m_front = static_cast<int32_t>(m_picts.size()) + add->get();
        for (auto file : add->getPicts()) {
            m_picts.push_back(file);
        }
        return true;
    }
    return false;
}

bool
PagingMode::hasNavigation()
{
    return m_picts.size() > 1;
}
