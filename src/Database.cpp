/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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
#include <format>
#include <map>

#include "Sqlite.hpp"
#include "Database.hpp"


namespace psc::sql {

PtrDatabase
Database::create(DbType dbType, const std::string& name)
{
    switch (dbType) {
    case DbType::Sqlite:
        return std::make_shared<SqlLite>(name);
    default:
        throw SqlException(std::format("The type {} is not implemeneted.", static_cast<int>(dbType)));
    }
}

SqlWhereBuilder*
SqlWhereBuilder::addOrderBy(const std::string order, OrderBy orderBy) {
    if (!m_order.empty()) {
        m_order += ',';
    }
    m_order += order;
    if (orderBy == OrderBy::Ascending) {
        m_order += " ASC";
    }
    else if (orderBy == OrderBy::Descending) {
        m_order += " DESC";
    }
    return this;
}

std::string
SqlWhereBuilder::getJoinExpr() {
    if (m_join == JoinBy::Or) {
        return {" OR "};
    }
    return {" AND "};
}

std::string
SqlWhereBuilder::buildTail() {
    std::string sql;
    sql.reserve(m_where.length() + m_order.length() + 32);
    if (!m_where.empty()) {
        sql += " WHERE ";
        sql += m_where;
    }
    if (!m_order.empty()) {
        sql += " ORDER BY ";
        sql += m_order;
    }
    return sql;
}

PtrSqlStatement
SqlSelectBuilder::build() {
    std::string sql("SELECT ");
    sql.reserve(m_select.length() + m_where.length() + m_order.length() + 128);
    if (!m_select.empty()) {
        sql += m_select;
    }
    else {
        sql += '*';
    }
    sql += " FROM ";
    sql += getTableName();
    sql += buildTail();
    return m_sqlLite->prepareStatement(sql);
}

PtrSqlStatement
SqlUpdateBuilder::build()
{
    std::string sql("UPDATE ");
    sql.reserve(m_update.length() + m_where.length() + m_order.length() + 128);
    sql += getTableName();
    sql += " SET ";
    sql += m_update;
    sql += buildTail();
    return m_sqlLite->prepareStatement(sql);
}

PtrSqlStatement
SqlDeleteBuilder::build()
{
    std::string sql("DELETE FROM ");
    sql.reserve(m_where.length() + m_order.length() + 64);
    sql += getTableName();
    sql += buildTail();
    return m_sqlLite->prepareStatement(sql);
}

PtrSqlStatement
SqlInsertBuilder::build() {
    std::string sql("INSERT INTO ");
    sql.reserve(m_insert.length() + m_insertCount*2 + 64);
    sql += getTableName();
    if (m_insert.empty()) {
        if (!m_table) {
            throw SqlException("The combination of using a tableName and no insert columns is not supported.");
        }
        for (auto &col : m_table->getColumns()) {
            addInsert(col->getName());
        }
    }
    sql += " (";
    sql += m_insert;
    sql += ") VALUES (";
    for (int i = 0; i < m_insertCount; ++i) {
        if (i > 0) {
            sql += ',';
        }
        sql += '?';
    }
    sql += ')';
    return m_sqlLite->prepareStatement(sql);
}


} /* namespace psc::sql */