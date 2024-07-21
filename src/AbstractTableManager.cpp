/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include <StringUtils.hpp>
#include <typeinfo>
#include <typeindex>
#include <string>   // to_string

#include "AbstractTableManager.hpp"
#include "TableProperties.hpp"

namespace psc {
namespace ui {


void
CustomColumn::build(bool autosizeColumns, AbstractTableManager* tableManager) const
{
    auto column = getColumn();
    // i'am a bit surprised this works ... (dynamic_cast does not see template dependency...)
    auto* treeCol = static_cast<Gtk::TreeModelColumn<std::any>*>(column);
    auto name = getName();
    auto table = tableManager->getTable();
    auto converter = getConverter();
    guint treCol;
    if (converter) {
        auto cellRendererText = Gtk::manage<Gtk::CellRendererText>(new Gtk::CellRendererText());
        treCol = table->append_column(name, *cellRendererText);
        auto* treeViewColumn = table->get_column(treCol-1);
        treeViewColumn->set_cell_data_func(
            *cellRendererText,
            sigc::mem_fun(*converter, &BaseConverter::convert));
    }
    else {
        treCol = table->append_column(name, *treeCol);
    }
    auto colIdx = treCol-1;
    // Set alignment
    auto treeViewColumn = table->get_column(colIdx);
    if (getHAlign() > 0.0f) {
        float hAlign,vAlign;
        Gtk::CellRenderer* cellRenderer = treeViewColumn->get_first_cell();
        cellRenderer->get_alignment(hAlign, vAlign);
        cellRenderer->set_alignment(getHAlign(), vAlign);
        // looks ugly sometimes
        //treeColumn->set_alignment(m_columnRecord->getHAlign(modIdx));
    }
    // as it seems this is not considered by default, but brings up auto sort on click
    //treeColumn->set_sort_column(modIdx);
    // listen to clicks to configure
    treeViewColumn->signal_clicked().connect(
            sigc::bind(
                      sigc::mem_fun(*tableManager, &AbstractTableManager::menu)
                    , treeCol));
    // handle sizing
    treeViewColumn->set_resizable(true);
    int colSize = static_cast<int>(tableManager->getColumnSize(colIdx));
    if (!autosizeColumns
     && colSize > 0) {
        // this works only mediocre if columns added the user must adjust
        //treeColumn->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
        treeViewColumn->set_fixed_width(colSize);   // this is a hint (won't break the ability to resize)
        //std::cout << "Column " << name << " set size " << colSize << std::endl;
    }
    else {
        treeViewColumn->set_expand(true);   // use to autosize
    }
    //treeColumn->set_reorderable(true);    prefere custom dlg
}

AbstractTableManager::AbstractTableManager(
               const std::shared_ptr<ColumnRecord>& columnRecord)
: m_columnRecord{columnRecord}
{
}

void
AbstractTableManager::setup(Gtk::Dialog* dlg)
{
    int width{0}, height{0};
    loadDialogSizeConfig(width, height);
    if (width > 0 && height > 0) {
        dlg->set_default_size(width, height);
    }
}

void
AbstractTableManager::setup(const Glib::RefPtr<Gtk::TreeView>& table)
{
    m_table = table;
    loadColumnIdxConfig(COLUMN_IDXS, m_visible);
    if (m_visible.empty()) {    // build some default, as tables without columns are useless
        for (int32_t i = 0; i < std::min(static_cast<int32_t>(m_columnRecord->getSize()), 4); ++i) {
            m_visible.push_back(i);
        }
    }
    loadColumnIdxConfig(COLUMN_SIZES, m_columnSizes);
    loadColumnSortConfig(m_sortModelIdx, m_sortType);
    build(false);
}


int
AbstractTableManager::columnNameToIndex(const Glib::ustring& columnName)
{
    int columnIdx = -1;
    if (!columnName.empty()) {
        for (guint i = 0; i < m_table->get_n_columns(); ++i) {
            auto treeColumn = m_table->get_column(i);
            if (treeColumn->get_title() == columnName) {
                columnIdx = i;
                break;
            }
        }
    }
    return columnIdx;
}

void
AbstractTableManager::saveConfig(const Gtk::Dialog* dlg)
{
    m_columnSizes = getTableColumnSizes();    // get the actual size into the model
    saveColumnIdxConfig(COLUMN_IDXS,  m_visible);
    saveColumnIdxConfig(COLUMN_SIZES, m_columnSizes);
    saveDialogSizeConfig(dlg->get_width(), dlg->get_height());
    saveColumnSortConfig(m_sortModelIdx, m_sortType);
}

Glib::RefPtr<Gtk::TreeView>
AbstractTableManager::getTable()
{
    return m_table;
}

size_t
AbstractTableManager::getColumnSize(guint colIdx)
{
    if (colIdx < m_visible.size()) {
        size_t modelIdx = m_visible[colIdx];
        return modelIdx < m_columnSizes.size()
                ? m_columnSizes[modelIdx]
                : 0;
    }
    return 0;
}

Gtk::Menu*
AbstractTableManager::create_popup(Gtk::TreeModelColumnBase* modelColumn)
{
    auto popupMenu = Gtk::make_managed<Gtk::Menu>();
    auto itemAscending = Gtk::make_managed<Gtk::MenuItem>("Sort _ascending", true);
    int32_t modelIdx = modelColumn->index();
    itemAscending->signal_activate().connect(
                sigc::bind(
                      sigc::mem_fun(*this, &AbstractTableManager::sort)
                    , modelIdx, Gtk::SortType::SORT_ASCENDING));
    popupMenu->append(*itemAscending);
    auto itemDescending = Gtk::make_managed<Gtk::MenuItem>("Sort _decending", true);
    itemDescending->signal_activate().connect(
                sigc::bind(
                      sigc::mem_fun(*this, &AbstractTableManager::sort)
                    , modelIdx, Gtk::SortType::SORT_DESCENDING));
    popupMenu->append(*itemDescending);
    auto itemProp = Gtk::make_managed<Gtk::MenuItem>("_Properties", true);
    itemProp->signal_activate().connect(
            sigc::mem_fun(*this, &AbstractTableManager::on_table_properties) );
    popupMenu->append(*itemProp);
    return popupMenu;
}

void
AbstractTableManager::menu(Gtk::TreeModelColumnBase* modelColumn)
{
    auto model = m_table->get_model();      // check if sortable
    if (auto sortmodel = Glib::RefPtr<Gtk::TreeSortable>::cast_dynamic(model)) {
        auto popupMenu = create_popup(modelColumn);
        popupMenu->show_all();
        popupMenu->attach_to_widget(*m_table.get());
        auto event = gtk_get_current_event();   // good ol c-gtk
        if (event != nullptr) {
            popupMenu->popup_at_pointer(event);
            g_free(event);
        }
        else {
            std::cout << "No event to popup...." << std::endl;
        }
    }
    else {  // if not sortable we can forward to properties
        on_table_properties();
    }
}

void
AbstractTableManager::sort(int32_t modelIdx, Gtk::SortType sortType)
{
    for (guint colIdx = 0;  colIdx < m_table->get_n_columns(); ++colIdx) {
        auto tableColumn = m_table->get_column(colIdx);
        if (modelIdx == static_cast<int32_t>(m_visible[colIdx])) {
            tableColumn->set_sort_indicator(true);
            tableColumn->set_sort_order(sortType);
            //std::cout << "AbstractTableManager::sort"
            //          << " col " << modelIdx
            //          << " sort " << (sortType == Gtk::SortType::SORT_ASCENDING ? "asc" : "desc") << std::endl;
            auto model = m_table->get_model();
            if (auto sortmodel = Glib::RefPtr<Gtk::TreeSortable>::cast_dynamic(model)) {
                sortmodel->set_sort_column(modelIdx, sortType);
            }
            m_sortModelIdx = modelIdx;
        }
        else {
            tableColumn->set_sort_indicator(false); // remove any other sort indicator
        }
    }
}


void
AbstractTableManager::on_table_properties()
{
    m_columnSizes = getTableColumnSizes();
    psc::ui::TableProperties* tableProp = TableProperties::show(m_columnRecord, m_visible);
    if (tableProp) {
        int ret = tableProp->run();
        if (ret == Gtk::RESPONSE_OK) {
            m_visible = tableProp->save();
            build(true);    // as this adds or removes columns use autosize
        }
        tableProp->hide();
        delete tableProp;
    }
}

std::vector<int32_t>
AbstractTableManager::getTableColumnSizes()
{
    std::vector<int32_t> columnSizes;
    for (guint recIdx = 0; recIdx < m_columnRecord->getSize(); ++recIdx) {
        Glib::ustring name =  m_columnRecord->getCustomColumn(recIdx).getName();
        int colIdx = columnNameToIndex(name);
        int32_t columnSize = 0;
        if (colIdx >= 0
         && colIdx < static_cast<int>(m_table->get_n_columns())) {
            auto treeColumn = m_table->get_column(colIdx);
            // only set width if manually sized
            columnSize = treeColumn->get_expand()
                            ? 0
                            : treeColumn->get_width();
        }
        columnSizes.push_back(columnSize);
    }
    return columnSizes;
}

/**
 * @param autosizeColumns true autosize columns, false use stored sizes
 */
void
AbstractTableManager::build(bool autosizeColumns)
{
    m_table->remove_all_columns();
    for (guint i = 0; i < m_visible.size(); ++i) {
        auto modIdx = m_visible[i];
        if (modIdx < static_cast<int32_t>(m_columnRecord->getSize())) {
            auto& customColumn = m_columnRecord->getCustomColumn(modIdx);
            customColumn.build(autosizeColumns, this);
        }
    }
    if (autosizeColumns) {
        m_table->columns_autosize();
    }
    sort(m_sortModelIdx, m_sortType);   // model sorting was preserved but to get the indicator right
    m_table->set_headers_clickable(true);
}



} /* end namespace ui */
} /* end namespace psc */