/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2024 RPf <gpl3@pfeifer-syscon.de>
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

#include <memory>
#include <vector>
#include <gtkmm.h>


#include "AbstractTableManager.hpp"

namespace psc {
namespace ui {

class ListEntry
: public Glib::Object
{
public:
    virtual ~ListEntry() = default;

    static Glib::RefPtr<ListEntry>
    create(const Glib::ustring& col, int modelIdx)
    {
        return Glib::RefPtr<ListEntry>(new ListEntry(col, modelIdx));
    }

    Glib::ustring getColumn() {
        return m_col;
    }
    int getModelIndex() {
        return m_modelIdx;
    }
private:
    ListEntry(const Glib::ustring& col, int modelIdx)
    : Glib::ObjectBase(typeid(ListEntry))
    , Glib::Object()
    , m_col{col}
    , m_modelIdx{modelIdx}
    {
    }
    Glib::ustring m_col;
    int m_modelIdx;
};

// have to simplify parameter passing as we run with the last function into some sigc++ limit ...
// what would be nice if the message was something better understandable than "have incompatible cv-qualifiers"
struct DragData {
    //DragData() = default;
    //virtual ~DragData() = default;
    const Glib::RefPtr<Gtk::ListBox>& srcList;
    const Glib::RefPtr<Gio::ListStore<ListEntry>>& srcModel;
    const Glib::RefPtr<Gtk::ListBox>& destList;
    const Glib::RefPtr<Gio::ListStore<ListEntry>> destModel;
    int index{0};
};

class TableProperties
: public Gtk::Dialog
{
public:
    TableProperties(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, const std::shared_ptr<ColumnRecord>& columnRecord, std::vector<int32_t>& visible);
    explicit TableProperties(const TableProperties& orig) = delete;
    virtual ~TableProperties() = default;

    std::vector<int32_t> save();
    static TableProperties* show(const std::shared_ptr<ColumnRecord>& columnRecord, std::vector<int32_t>& visible);
protected:
    bool pressEvent(GdkEventButton* event);
    bool pressCheck(GdkEventButton* event
            , const DragData& dragData);
    bool releaseEvent(GdkEventButton* event);
    bool releaseCheck(GdkEventButton* event
            , const DragData& dragData);

    Gtk::Widget* create_label(const Glib::RefPtr<Glib::Object>& item);
    static constexpr auto TRANSFER_BITS = 8;
private:
    std::shared_ptr<DragData> m_dragDataStart;
    Glib::RefPtr<Gtk::ListBox> m_availList;
    Glib::RefPtr<Gtk::ListBox> m_usedList;
    std::shared_ptr<ColumnRecord> m_columnRecord;
    Glib::RefPtr<Gio::ListStore<ListEntry>> m_availModel;
    Glib::RefPtr<Gio::ListStore<ListEntry>> m_usedModel;
    static constexpr auto RESOURCE_PREFIX= "/de/pfeifer_syscon/genricimg";
};



} /* end namespace ui */
} /* end namespace psc */