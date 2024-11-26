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
#include <string_view>
#include <gtkmm.h>
#include <any>      // std::any

namespace psc {
namespace ui {


class BaseConverter {
public:
    BaseConverter() = default;
    virtual ~BaseConverter() = default;

    virtual void convert(Gtk::CellRenderer* rend, const Gtk::TreeModel::iterator& iter) = 0;
};

template<typename T>
class CustomConverter
: public BaseConverter
{
public:
    CustomConverter(Gtk::TreeModelColumn<T>& col)
    : m_col{col}
    {
    }
    virtual ~CustomConverter() = default;
    virtual void convert(Gtk::CellRenderer* rend, const Gtk::TreeModel::iterator& iter) override = 0;
    Gtk::TreeModelColumn<T>& getColumn() {
        return m_col;
    }
protected:
    Gtk::TreeModelColumn<T>& m_col;

};

class AbstractTableManager;

// collect all the extra infos for a column
class CustomColumn
{
public:
    CustomColumn(
              const char* name
            , Gtk::TreeModelColumnBase* column
            , const std::shared_ptr<BaseConverter>& converter
            , float halign)
    : m_name{name}
    , m_column{column}
    , m_convert{converter}
    , m_halign{halign}
    {
    }
    virtual ~CustomColumn() = default;
    Glib::ustring getName() const {
        return m_name;
    }
    Gtk::TreeModelColumnBase* getColumn() const {
        return m_column;
    }
    const std::shared_ptr<BaseConverter>& getConverter() const  {
        return m_convert;
    }
    float getHAlign() const  {
        return m_halign;
    }
    void build(bool autosizeColumn, AbstractTableManager* tableManager) const;

private:
    Glib::ustring m_name;
    // using pointer should be safe as the instances are kept internally as well
    Gtk::TreeModelColumnBase* m_column;
    std::shared_ptr<BaseConverter> m_convert;
    float m_halign;

};

class ColumnRecord
: public Gtk::TreeModel::ColumnRecord
{
public:
    ColumnRecord()
    : Gtk::TreeModel::ColumnRecord() {

    }
    explicit ColumnRecord(const ColumnRecord& orig) = delete;
    virtual ~ColumnRecord() = default;

    template<typename T>
    void add(const char* name
            , Gtk::TreeModelColumn<T>& col
            , float halign = 0.0f) {
        std::shared_ptr<BaseConverter> baseConverter;
        add(col, name, baseConverter, halign);
    }
    template<typename T>
    void add(const char* name
            , const std::shared_ptr<CustomConverter<T>>& customConvert
            , float halign = 0.0f) {
        auto& col = customConvert->getColumn();
        add(col, name, customConvert, halign);
    }
    const CustomColumn& getCustomColumn(size_t idx) const {
        return m_columns[idx];
    }
    size_t getSize() const {
        return m_columns.size();
    }
protected:
    template<typename T>
    void add( Gtk::TreeModelColumn<T>& col
            , const char* name
            , const std::shared_ptr<BaseConverter>& baseConvert
            , float halign) {
        Gtk::TreeModel::ColumnRecord::add(col);
        m_columns.push_back(CustomColumn(
                  name
                , &col
                , baseConvert
                , halign));
    }

    std::vector<CustomColumn> m_columns;
};

class AbstractTableManager
{
public:
    AbstractTableManager(const std::shared_ptr<ColumnRecord>& columnRecord);
    explicit AbstractTableManager(const AbstractTableManager& orig) = delete;
    virtual ~AbstractTableManager() = default;

    void setup(Gtk::Dialog* dlg);
    void setup(const Glib::RefPtr<Gtk::TreeView>& table);
    void build(bool autosizeColumns);
    void menu(Gtk::TreeModelColumnBase* modelColumn);
    void saveConfig(const Gtk::Dialog* dlg);
    Glib::RefPtr<Gtk::TreeView> getTable();
    size_t getColumnSize(guint colIdx);
    /**
     *  Allow disable sorting
     *   e.g. for large models and converted data
     *   this might be too time consuming
     *     (maybe it will be more efficient if we implement sort for raw data)
     */
    void setAllowSort(bool allowSort);
    bool getAllowSort();

    virtual void loadColumnIdxConfig(const Glib::ustring& keyName, std::vector<int32_t>& colmns) = 0;
    virtual void loadDialogSizeConfig(int& dlgWidth, int& dlgHeight) = 0;
    virtual void loadColumnSortConfig(int& sortColumnIdx, Gtk::SortType& sortType) = 0;
    virtual void saveColumnIdxConfig(const Glib::ustring& keyName, const std::vector<int32_t>& config) = 0;
    virtual void saveDialogSizeConfig(int dlgWidth, int dlgHeight) = 0;
    virtual void saveColumnSortConfig(int sortColumnIdx, Gtk::SortType sortType) = 0;

protected:
    std::vector<int32_t> getTableColumnSizes();
    int columnNameToIndex(const Glib::ustring& m_sortColumn);

    Gtk::Menu* create_popup(Gtk::TreeModelColumnBase* modelColumn);
    void on_table_properties();
    void sort(int32_t modelIdx, Gtk::SortType sortType);

    std::shared_ptr<ColumnRecord> m_columnRecord;
    std::vector<int32_t> m_visible;
    std::vector<int32_t> m_columnSizes;  // indexed by columnRecord index
    Glib::RefPtr<Gtk::TreeView> m_table;
    int32_t m_sortModelIdx;
    Gtk::SortType m_sortType;
    bool m_allowSort{true};

    static constexpr auto COLUMN_IDXS = "columnIdxs";
    static constexpr auto COLUMN_SIZES = "columnSizes";
};



} /* end namespace ui */
} /* end namespace psc */
