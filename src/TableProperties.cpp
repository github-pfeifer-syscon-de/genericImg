/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2024 RPf 
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

#include "TableProperties.hpp"

namespace psc {
namespace ui {


TableProperties::TableProperties(
              BaseObjectType* cobject
            , const Glib::RefPtr<Gtk::Builder>& builder
            , const std::shared_ptr<ColumnRecord>& columnRecord
            , std::vector<int32_t>& visible)
: Gtk::Dialog{cobject}
, m_columnRecord{columnRecord}
{
    auto object = builder->get_object("avail");
    m_availList = Glib::RefPtr<Gtk::ListBox>::cast_dynamic(object);
    object = builder->get_object("used");
    m_usedList = Glib::RefPtr<Gtk::ListBox>::cast_dynamic(object);
    m_availModel = Gio::ListStore<ListEntry>::create();
    m_usedModel = Gio::ListStore<ListEntry>::create();
    std::set<int32_t> usedSet;
    for (size_t j = 0; j < visible.size(); ++j) {
        if (visible[j] < static_cast<int32_t>(m_columnRecord->getSize())) {
            auto customColumn = m_columnRecord->getCustomColumn(visible[j]);
            auto name = customColumn.getName();
            m_usedModel->append(ListEntry::create(name, visible[j]));
            usedSet.insert(visible[j]);
        }
    }
    for (size_t i = 0; i < m_columnRecord->getSize(); ++i) {
        auto customColumn = m_columnRecord->getCustomColumn(i);
        auto name = customColumn.getName();
        if (!usedSet.contains(static_cast<int32_t>(i))) {
            m_availModel->append(ListEntry::create(name, static_cast<int32_t>(i)));
        }
    }
    m_availList->bind_model(m_availModel, sigc::mem_fun(*this, &TableProperties::create_label));
    m_usedList->bind_model(m_usedModel, sigc::mem_fun(*this, &TableProperties::create_label));

    m_availList->signal_button_press_event().connect(
                sigc::mem_fun(*this, &TableProperties::pressEvent));
    m_availList->signal_button_release_event().connect(
                sigc::mem_fun(*this, &TableProperties::releaseEvent));
    //m_availList->add_events(Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK );
    m_usedList->signal_button_press_event().connect(
                sigc::mem_fun(*this, &TableProperties::pressEvent));
    m_usedList->signal_button_release_event().connect(
                sigc::mem_fun(*this, &TableProperties::releaseEvent));
    //m_usedList->add_events(Gdk::EventMask::BUTTON_PRESS_MASK | Gdk::EventMask::BUTTON_RELEASE_MASK );

// does not allow interchangable transfere
//    std::vector<Gtk::TargetEntry> drag_targets = {
//                                        Gtk::TargetEntry("STRING"),
//                                        Gtk::TargetEntry("text/plain")};
//
//    m_availList->drag_source_set(drag_targets);
//    m_availList->drag_dest_set(drag_targets);
//    m_usedList->drag_source_set(drag_targets);
//    m_usedList->drag_dest_set(drag_targets);
//
//    m_availList->signal_drag_begin().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::drag_begin)
//                , left2right));
//    m_availList->signal_drag_data_get().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::source_get_data)
//                , left2right));
//    m_usedList->signal_drag_data_received().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::destination_received_data)
//                , left2right));
//
//    DragData right2left {   // allow transfere in any direction
//        .srcList{m_usedList}
//        ,.srcModel{m_usedModel}
//        ,.destList{m_availList}
//        ,.destModel{m_availModel}
//    };
//    m_availList->signal_drag_begin().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::drag_begin)
//                , right2left));
//    m_availList->signal_drag_data_get().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::source_get_data)
//                , right2left));
//    m_usedList->signal_drag_data_received().connect(
//            sigc::bind(
//                  sigc::mem_fun(*this, &TableProperties::destination_received_data)
//                , right2left));
}

bool
TableProperties::pressCheck(
          GdkEventButton* event
        , const DragData& dragData)
{
        int x1,y1;
        Gdk::ModifierType modifier;
        auto devRef = Glib::wrap(event->device, true);  // without true we kill the event device ...
        auto window = dragData.srcList->get_window();
        window->get_device_position(devRef, x1, y1, modifier);
        auto alloc = dragData.srcList->get_allocation();
        if (x1 >= alloc.get_x()
         && x1 < alloc.get_width()
         && y1 >= alloc.get_y()
         && y1 < alloc.get_height()) {
            Gtk::ListBoxRow* row = dragData.srcList->get_row_at_y(y1 - alloc.get_y());
            if (row) {
                dragData.srcList->select_row(*row);
                dragData.destList->unselect_all();
                m_dragDataStart = std::make_shared<DragData>(dragData);
                m_dragDataStart->index = row->get_index();
                auto sec_window = dragData.destList->get_window();
                auto cursor = Gdk::Cursor::create(window->get_display(), "move");        //
                if (cursor) {
                    window->set_cursor(cursor);
                    sec_window->set_cursor(cursor);
                }
                //std::cout << "TableProperties::pressEvent"
                //          << " srcRow " << row->get_index()
                //          << " from " << (dragData.srcModel == m_availModel ? "avail" : "used")
                //          << std::endl;
            }
            else {
                std::cout << "TableProperties::drag_begin no row" << std::endl;
            }
        return true;
        }
        return false;
}


bool
TableProperties::pressEvent(GdkEventButton* event)
{
    if (event->button == 1) {
        DragData left2right {
            .srcList{m_availList}
            ,.srcModel{m_availModel}
            ,.destList{m_usedList}
            ,.destModel{m_usedModel}
        };
        if (pressCheck(event, left2right)) {
            return true;
        }
        DragData right2left {   // allow transfere in any direction
            .srcList{m_usedList}
            ,.srcModel{m_usedModel}
            ,.destList{m_availList}
            ,.destModel{m_availModel}
        };
        if (pressCheck(event, right2left)) {
            return true;
        }
    }
    return false;
}

bool
TableProperties::releaseCheck(GdkEventButton* event
        , const DragData& dragData)
{
    int x1,y1;
    Gdk::ModifierType modifier;
    auto devRef = Glib::wrap(event->device, true);
    dragData.srcList->get_window()->get_device_position(devRef, x1, y1, modifier);
    auto alloc = dragData.srcList->get_allocation();
    if (x1 >= alloc.get_x()
     && x1 < alloc.get_width()
     && y1 >= alloc.get_y()
     && y1 < alloc.get_height()) {
        auto destRow = dragData.srcList->get_row_at_y(y1 - alloc.get_y());
        //std::cout << "TableProperties::releaseEvent"
        //          << " destRow " << destRow
        //          << " from " << (m_dragDataStart->srcModel == m_availModel ? "avail" : "used")
        //          << " into " << (dragData.srcModel == m_availModel ? "avail" : "used")
        //          << std::endl;
        auto srcRow = m_dragDataStart->srcModel->get_item(m_dragDataStart->index);
        guint insertRow = destRow != nullptr
                            ? destRow->get_index()
                            : dragData.srcModel->get_n_items();
        dragData.srcModel->insert(insertRow, srcRow);
        guint removeRow = m_dragDataStart->index;
        if (dragData.srcModel == m_dragDataStart->srcModel
         && insertRow <= removeRow) {    // adjust for changed position on moving in same model
            ++removeRow;
        }
        m_dragDataStart->srcModel->remove(removeRow);   // remove as last as we may insert into same list
        return true;
    }
    else {
        //std::cout << "TableProperties::releaseEvent"
        //          << " into " << (dragData.srcModel == m_availModel ? "avail" : "used")
        //          << " coord " << x1 << "," << y1
        //          << " outside " << alloc.get_x() << "," << alloc.get_width()
        //          << " " << alloc.get_y() << "," << alloc.get_height()
        //          << std::endl;
    }
    return false;
}

bool
TableProperties::releaseEvent(GdkEventButton* event)
{
    //std::cout << "TableProperties::releaseEvent"
    //          << " typ " << event->type
    //          << " btn " << event->button
    //          << " x " << event->x << "," << event->y
    //          << std::endl;
    if (event->button == 1
     && m_dragDataStart) {
        bool ret = false;
        DragData left2right {
            .srcList{m_availList}
            ,.srcModel{m_availModel}
            ,.destList{m_usedList}
            ,.destModel{m_usedModel}
        };
        ret = releaseCheck(event, left2right);
        if (!ret) {
            DragData right2left {   // allow transfere in any direction
                .srcList{m_usedList}
                ,.srcModel{m_usedModel}
                ,.destList{m_availList}
                ,.destModel{m_availModel}
            };
            ret = releaseCheck(event, right2left);
        }
        auto window = m_usedList->get_window();
        auto sec_window = m_availList->get_window();
        window->set_cursor();       // indicate we are done
        sec_window->set_cursor();
        m_dragDataStart.reset();
        return true;
    }
    return false;
}

Gtk::Widget*
TableProperties::create_label(const Glib::RefPtr<Glib::Object>& item)
{
    if (auto column = Glib::RefPtr<ListEntry>::cast_dynamic(item)) {
        auto widget = Gtk::make_managed<Gtk::Label>(column->getColumn());
        widget->set_visible(true);
        return widget;
    }
    else {
        std::cout << "item not of expected type " << typeid(item).name() << std::endl;
    }
    return nullptr;
}

//void
//TableProperties::drag_begin(
//              const Glib::RefPtr<Gdk::DragContext>& context
//            , const DragData& dragData)
//{
//    int x1,y1;
//    Gdk::ModifierType modifier;
//    dragData.srcList->get_window()->get_device_position(context->get_device(), x1, y1, modifier);
//    auto alloc = dragData.srcList->get_allocation();
//    if (x1 >= alloc.get_x()
//     && x1 < alloc.get_width()
//     && y1 >= alloc.get_y()
//     && y1 < alloc.get_height()) {
//        Gtk::ListBoxRow* row = dragData.srcList->get_row_at_y(y1 - alloc.get_y());
//        if (row) {
//            dragData.srcList->select_row(*row);
//            dragData.destList->unselect_all();
//        }
//        else {
//            std::cout << "TableProperties::drag_begin no row" << std::endl;
//        }
//    }
//    else {
//        std::cout << "TableProperties::drag_begin not in widget"
//                  << " x1 " << x1 << " y1 " << y1
//                  << " alloc.x " << alloc.get_x() << " alloc.y " << alloc.get_y()
//                  << " alloc.w " << alloc.get_width() << " alloc.h " << alloc.get_height()
//                  << std::endl;
//    }
//}
//
//
//void
//TableProperties::source_get_data(
//              const Glib::RefPtr<Gdk::DragContext>& context
//            , Gtk::SelectionData& selection_data
//            , guint info
//            , guint time
//            , const DragData& dragData)
//{
//    auto row = dragData.srcList->get_selected_row();
//    if (row != nullptr) {
//        auto listEntry = dragData.srcModel->get_item(row->get_index());
//        if (listEntry) {
//            std::string source_message = listEntry->getColumn();
//            selection_data.set_text(source_message);
//            //selection_data.set(
//            //        selection_data.get_target(),
//            //        TRANSFER_BITS /* 8 bits format */,
//            //        (const guchar*)source_message.c_str(),
//            //        source_message.size());
//        }
//        else {
//            std::cout << "Not found entry from row " << row << " in availList" << std::endl;
//        }
//    }
//    else {
//        std::cout << "Not found label from row in availList" << std::endl;
//    }
//}
//
//void
//TableProperties::destination_received_data(
//              const Glib::RefPtr<Gdk::DragContext>& context
//            , int x
//            , int y
//            , const Gtk::SelectionData& selection_data
//            , guint info
//            , guint time
//            , const DragData& dragData)
//{
//    if ((selection_data.get_length() >= 0)
//     && (selection_data.get_format() == TRANSFER_BITS)) {
//        std::string message = selection_data.get_data_as_string();
//        //std::cout << message << " x " << x << " y " << y << std::endl;
//        int x1,y1;
//        Gdk::ModifierType modifier;
//        auto alloc = dragData.destList->get_allocation();
//        dragData.destList->get_window()->get_device_position(context->get_device(), x1, y1, modifier);
//        //m_usedList->get_pointer(x1, y1);
//        if (x1 >= alloc.get_x()
//         && x1 < alloc.get_width()
//         && y1 >= alloc.get_y()
//         && y1 < alloc.get_height()) {
//            auto destRow = dragData.destList->get_row_at_y(y1 - alloc.get_y());
//            for (guint i = 0; i < dragData.srcModel->get_n_items(); ++i) {
//                auto listEntry = dragData.srcModel->get_item(i);
//                if (message == listEntry->getColumn()) {
//                    dragData.destModel->insert(destRow != nullptr
//                                        ? destRow->get_index()
//                                        : dragData.destModel->get_n_items(), listEntry);
//                    dragData.srcModel->remove(i);
//                    break;
//                }
//            }
//        }
//        else {
//            std::cout << "No coords in used list"
//                      << " x " << x1 << " y " << y1 << std::endl;
//        }
//    }
//
//    context->drag_finish(false, false, time);
//}


std::vector<int32_t>
TableProperties::save()
{
    std::vector<int32_t> visible;
    for (guint i = 0; i < m_usedModel->get_n_items(); ++i) {
        Glib::RefPtr<ListEntry> entry = m_usedModel->get_item(i);
        visible.push_back(entry->getModelIndex());
    }
    if (visible.empty()) {
        visible.push_back(0);   // at least one column
    }
    return visible;
}

TableProperties*
TableProperties::show(const std::shared_ptr<ColumnRecord>& columnRecord, std::vector<int32_t>& visible)
{
    psc::ui::TableProperties* tableProp = nullptr;
    auto refBuilder = Gtk::Builder::create();
    try {
        auto gappl = Gtk::Application::get_default();
        auto appl = Glib::RefPtr<Gtk::Application>::cast_dynamic(gappl);
        refBuilder->add_from_resource(
            std::string(RESOURCE_PREFIX) + "/table_properties.ui");
        refBuilder->get_widget_derived("table-prop-dlg", tableProp, columnRecord, visible);
        if (tableProp) {
            tableProp->set_transient_for(*appl->get_active_window());
        }
        else {
            std::cerr << "TableProperties::show(): No \"table-prop-dlg\" object in table_properties" << std::endl;
        }
    }
    catch (const Glib::Error& ex) {
        std::cerr << "TableProperties::show(): " << ex.what() << std::endl;
    }
    return tableProp;
}


} /* end namespace ui */
} /* end namespace psc */
