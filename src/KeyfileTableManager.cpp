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

#include "KeyfileTableManager.hpp"

namespace psc {
namespace ui {


KeyfileTableManager::KeyfileTableManager(
          const std::shared_ptr<ColumnRecord>& columnRecord
        , Glib::KeyFile* keyFile
        , const char* group)
: AbstractTableManager{columnRecord}
, m_keyFile{keyFile}
, m_group{group}
{
}

/**
 * @param just load the named set of indexes
 */
void
KeyfileTableManager::loadColumnIdxConfig(const Glib::ustring& keyName, std::vector<int32_t>& columnIdx)
{
    columnIdx.clear();
    if (m_keyFile->has_group(m_group)) {
        if (m_keyFile->has_key(m_group, keyName)) {
            auto config = m_keyFile->get_string(m_group, keyName);
            std::vector<Glib::ustring> split;
            StringUtils::split(config, DELIM, split);
            for (auto& part : split) {
                auto modIdx = std::stoi(part);
                columnIdx.push_back(modIdx);
            }
        }
    }
}


void
KeyfileTableManager::loadDialogSizeConfig(int& dlgWidth, int& dlgHeight)
{
    if (m_keyFile->has_group(m_group)) {
        if (m_keyFile->has_key(m_group, DIALOG_WIDTH)
         && m_keyFile->has_key(m_group, DIALOG_HEIGHT)) {
            dlgWidth = m_keyFile->get_integer(m_group, DIALOG_WIDTH);
            dlgHeight = m_keyFile->get_integer(m_group, DIALOG_HEIGHT);
        }
    }
}

void
KeyfileTableManager::loadColumnSortConfig(int& sortColumnIdx, Gtk::SortType& sortType)
{
    sortColumnIdx = -1; // ensure some default
    if (m_keyFile->has_group(m_group)) {
        if (m_keyFile->has_key(m_group, COLUMN_SORTIDX)
         && m_keyFile->has_key(m_group, COLUMN_SORTTYPE)) {
            sortColumnIdx = m_keyFile->get_integer(m_group, COLUMN_SORTIDX);
            sortType = m_keyFile->get_boolean(m_group, COLUMN_SORTTYPE)
                        ? Gtk::SortType::SORT_ASCENDING
                        : Gtk::SortType::SORT_DESCENDING;
        }
    }
}

void
KeyfileTableManager::saveColumnIdxConfig(const Glib::ustring& keyName, const std::vector<int32_t>& config)
{
    Glib::ustring conf;
    for (auto idx : config) {
        if (!conf.empty()) {
            conf += DELIM;
        }
        conf += std::to_string(idx);
    }
    m_keyFile->set_string(m_group, keyName, conf);
}

void
KeyfileTableManager::saveDialogSizeConfig(int dlgWidth, int dlgHeight)
{
    m_keyFile->set_integer(m_group, DIALOG_WIDTH, dlgWidth);
    m_keyFile->set_integer(m_group, DIALOG_HEIGHT, dlgHeight);
}

void
KeyfileTableManager::saveColumnSortConfig(int sortColumnIdx, Gtk::SortType sortType)
{
    m_keyFile->set_integer(m_group, COLUMN_SORTIDX, sortColumnIdx);
    m_keyFile->set_boolean(m_group, COLUMN_SORTTYPE, sortType == Gtk::SortType::SORT_ASCENDING);
}

} /* end namespace ui */
} /* end namespace psc */
