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

#pragma once
#include <memory>
#include <vector>
#include <string_view>
#include <gtkmm.h>

#include "AbstractTableManager.hpp"

namespace psc {
namespace ui {

class KeyfileTableManager
: public AbstractTableManager
{
public:
    KeyfileTableManager(
          const std::shared_ptr<ColumnRecord>& columnRecord
        , Glib::KeyFile* keyFile
        , const char* cfgGroup);
    explicit KeyfileTableManager(const KeyfileTableManager& orig) = delete;
    virtual ~KeyfileTableManager() = default;

    void loadColumnIdxConfig(const Glib::ustring& keyName, std::vector<int32_t>& colmns) override;
    void loadDialogSizeConfig(int& dlgWidth, int& dlgHeight) override;
    void loadColumnSortConfig(int& sortColumnIdx, Gtk::SortType& sortType) override;
    void saveColumnIdxConfig(const Glib::ustring& keyName, const std::vector<int32_t>& config) override;
    void saveDialogSizeConfig(int dlgWidth, int dlgHeight) override;
    void saveColumnSortConfig(int sortColumnIdx, Gtk::SortType sortType) override;

private:
    static constexpr auto COLUMN_SORTIDX = "columnSortIdx";
    static constexpr auto COLUMN_SORTTYPE = "columnSortType";
    static constexpr auto DIALOG_WIDTH = "dialogWidth";
    static constexpr auto DIALOG_HEIGHT = "dialogHeight";
    Glib::KeyFile* m_keyFile;
    const Glib::ustring m_group;
    const Glib::ustring m_key;
    static constexpr auto DELIM = ';';

};

} /* end namespace ui */
} /* end namespace psc */
