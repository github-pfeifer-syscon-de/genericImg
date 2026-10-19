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
#include <string>
#include <sqlite3.h>
#include <exception>

#include "Sqlite.hpp"

#undef DEBUG_SQLLITE

namespace psc::sql {
SqlException::SqlException(const std::string& error)
: std::runtime_error(error)
{
}

std::string
SqlIndex::getCreate(const SqlTable* table) const
{
    std::string stmt;
    stmt.reserve(256);
    stmt += "CREATE ";
    if (m_unique) {
        stmt += "UNIQUE ";
    }
    stmt += "INDEX IF NOT EXISTS ";
    stmt += m_name;
    stmt += " ON ";
    stmt += table->getName();
    stmt += " (";
    int n{};
    for (auto& column : m_columns) {
        stmt += '\n';
        if (n > 0) {
            stmt += ',';
        }
        stmt += column->getName();
        ++n;
    }
    stmt += ");\n";
    return stmt;
}

std::string
SqlTable::getCreate() const
{
    std::string stmt;
    stmt.reserve(512);
    stmt += "CREATE TABLE IF NOT EXISTS ";
    stmt += m_name;
    stmt += " (";
    int n{};
    for (auto& column : m_columns) {
        stmt += '\n';
        if (n > 0) {
            stmt += ',';
        }
        stmt += column->getCreate();
        ++n;
    }
    stmt += ");\n";
    for (auto& indx : m_indexes) {
        stmt += indx->getCreate(this);
    }
    return stmt;
}

std::string
SqlTable::getInsert() const
{
    std::string stmt;
    stmt.reserve(256);
    stmt += "INSERT INTO ";
    stmt += m_name;
    stmt += " (";
    size_t n{};
    for (auto& column : m_columns) {
        if (n > 0) {
            stmt += ',';
        }
        stmt += column->getName();
        ++n;
    }
    stmt += ") VALUES (";
    for (n = 0; n < m_columns.size(); ++n) {
        if (n > 0) {
            stmt += ',';
        }
        stmt += '?';
    }
    stmt += ");";
    return stmt;
}

std::string
SqlTable::getSelect(const std::string& fields) const
{
    std::string stmt;
    stmt.reserve(32 + fields.size() + m_name.size());
    stmt += "SELECT ";
    stmt += fields;
    stmt += " FROM ";
    stmt += m_name;
    return stmt;
}

std::string
SqlTable::getUpdate(const std::vector<PtrSqlColumn>& cols) const
{
    std::string stmt;
    stmt.reserve(256);
    stmt += "UPDATE ";
    stmt += m_name;
    stmt += " SET ";
    size_t n{};
    for (auto& column : cols) {
        if (n > 0) {
            stmt += ',';
        }
        stmt += column->getName();
        stmt += "=?";
        ++n;
    }
    return stmt;
}

std::string
SqlTable::getDeleteFrom() const
{
    std::string stmt;
    stmt.reserve(64);
    stmt += "DELETE FROM ";
    stmt += m_name;
    return stmt;
}

SqlLiteStmt::SqlLiteStmt(SqlLite* sqlLite, const std::string& sql)
: m_sqlLite{sqlLite}
, m_sql{sql}
{
#   ifdef DEBUG_SQLLITE
    std::cout << "SqlLiteStmt::SqlLiteStmt " << sql << std::endl;
#   endif
    auto rc = sqlite3_prepare_v3(
        m_sqlLite->getDb()
        , sql.c_str()
        , -1
        , 0
        , &m_stmt
        , nullptr);
    if (rc != SQLITE_OK) {
        throw SqlException(std::format("SqlLiteStmt::SqlLiteStmt error {} executing {}", sqlite3_errmsg(m_sqlLite->getDb()), m_sql));
    }
}

SqlLiteStmt::~SqlLiteStmt()
{
    if (m_stmt != nullptr) {
        auto rc = sqlite3_finalize(m_stmt);
        if (rc != SQLITE_OK) {
            std::cout << "SqlLiteStmt::~SqlLiteStmt error " << rc << " finalize stmt" << std::endl;
        }
        m_stmt = nullptr;
    }
}

void
SqlLiteStmt::reset()
{
    sqlite3_reset(m_stmt);
}

bool
SqlLiteStmt::next()
{
    auto rc = sqlite3_step(m_stmt);
    if (rc == SQLITE_ERROR) {
        throw SqlException(std::format("SQL error {} next executing {}", sqlite3_errmsg(m_sqlLite->getDb()), m_sql));
    }
    return rc == SQLITE_ROW;
}

int
SqlLiteStmt::exec()
{
    auto rc = sqlite3_step(m_stmt);
    if (rc != SQLITE_OK && rc != SQLITE_CONSTRAINT && rc != SQLITE_DONE) {
        throw SqlException(std::format("SQL error {} exec executing {}", sqlite3_errmsg(m_sqlLite->getDb()), m_sql));
    }
    sqlite3_reset(m_stmt);  // make stmt ready for next bind
    return rc;
}


int64_t
SqlLiteStmt::getRowCount()
{
    return sqlite3_data_count(m_stmt);
}

std::vector<int>
SqlLiteStmt::getResultTypes()
{
    std::vector<int> ret;
    auto colCnt = sqlite3_column_count(m_stmt);
    ret.reserve(static_cast<size_t>(colCnt));
    for (int col = 0; col < colCnt; ++col) {
        auto type = sqlite3_column_type(m_stmt, col);
        ret.push_back(type);
    }
    return ret;
}

std::vector<std::string>
SqlLiteStmt::getColumnNames()
{
    std::vector<std::string> ret;
    auto colCnt = sqlite3_column_count(m_stmt);
    ret.reserve(static_cast<size_t>(colCnt));
    for (int i = 0; i < colCnt; ++i) {
        auto name = sqlite3_column_name(m_stmt, i);
        ret.emplace_back(std::move(std::string(name)));
    }
    return ret;
}

std::string
SqlLiteStmt::getString(int col)
{
    if (sqlite3_column_type(m_stmt, col) == SQLITE_NULL) {
        return "";
    }
    return {reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, col))};
}

double
SqlLiteStmt::getDouble(int col)
{
    if (sqlite3_column_type(m_stmt, col) == SQLITE_NULL) {
        return 0.0;
    }
    return sqlite3_column_double(m_stmt, col);
}

int64_t
SqlLiteStmt::getLong(int col)
{
    if (sqlite3_column_type(m_stmt, col) == SQLITE_NULL) {
        return 0l;
    }
    return sqlite3_column_int64(m_stmt, col);
}

int
SqlLiteStmt::getColumnIdx(const std::string& name)
{
    if (m_nameCols.empty()) {
        auto names = getColumnNames();
        for (size_t n = 0; n < names.size(); ++n) {
            m_nameCols.insert(std::pair(names[n], static_cast<int>(n)));
        }
    }
    auto itr = m_nameCols.find(name);
    if (itr == m_nameCols.end()) {
        throw SqlException(std::format("The requested column name {} was not found!", name ));
    }
    return itr->second;
}

std::string
SqlLiteStmt::getString(const std::string& colName)
{
    auto n = getColumnIdx(colName);
    return getString(n);
}

double
SqlLiteStmt::getDouble(const std::string& colName)
{
    auto n = getColumnIdx(colName);
    return getDouble(n);
}

int64_t
SqlLiteStmt::getLong(const std::string& colName)
{
    auto n = getColumnIdx(colName);
    return getLong(n);
}

void
SqlLiteStmt::setString(int bindIdx, const std::string& value)
{
    if (bindIdx <= 0) {
        throw SqlException(std::format("SqlLiteStmt::setString idx {} value must be > 0", bindIdx));
    }
    auto rc = sqlite3_bind_text(m_stmt, bindIdx, value.c_str(), -1, SQLITE_TRANSIENT); // transient sqllite will copy
    if (rc != SQLITE_OK) {
        throw SqlException(std::format("SqlLiteStmt::setString idx {} error {}", bindIdx, sqlite3_errmsg(m_sqlLite->getDb())));
    }
}

void
SqlLiteStmt::setDouble(int bindIdx, double value)
{
    if (bindIdx <= 0) {
        throw SqlException(std::format("SqlLiteStmt::setDouble idx {} value must be > 0", bindIdx));
    }
    auto rc = sqlite3_bind_double(m_stmt, bindIdx, value);
    if (rc != SQLITE_OK) {
        throw SqlException(std::format("SqlLiteStmt::setDouble idx {} error {}", bindIdx, sqlite3_errmsg(m_sqlLite->getDb())));
    }
}

void
SqlLiteStmt::setLong(int bindIdx, int64_t value)
{
    if (bindIdx <= 0) {
        throw SqlException(std::format("SqlLiteStmt::setLong idx {} value must be > 0", bindIdx));
    }
    auto rc = sqlite3_bind_int64(m_stmt, bindIdx, value);
    if (rc != SQLITE_OK) {
        throw SqlException(std::format("SqlLiteStmt::setLong idx {} error {}", bindIdx, sqlite3_errmsg(m_sqlLite->getDb())));
    }
}

void SqlLiteStmt::setNull(int bindIdx)
{
    if (bindIdx <= 0) {
        throw SqlException(std::format("SqlLiteStmt::setNull idx {} value must be > 0", bindIdx));
    }
    auto rc = sqlite3_bind_null(m_stmt, bindIdx);
    if (rc != SQLITE_OK) {
        throw SqlException(std::format("SqlLiteStmt::setNull idx {} error {}", bindIdx, sqlite3_errmsg(m_sqlLite->getDb())));
    }
}


SqlLite::SqlLite(const std::string& name)
{
    auto rc = sqlite3_open(name.c_str(), &m_db);
    if (rc != SQLITE_OK) {
         throw SqlException(std::format("Can't open database: {}", sqlite3_errmsg(m_db)));
    }
}

SqlLite::~SqlLite()
{
    if (m_db != nullptr) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

std::unique_ptr<SqlLiteStmt>
SqlLite::prepareStatement(const std::string& stmt)
{
    return std::make_unique<SqlLiteStmt>(this, stmt);
}

void
SqlLite::beginTransaction()
{
    execute("BEGIN TRANSACTION");
}

void
SqlLite::commit()
{
    execute("COMMIT");
}

void SqlLite::rollback()
{
    execute("ROLLBACK");
}

void
SqlLite::create(const PtrSqlTable& table)
{
    execute(table->getCreate());
}

std::unique_ptr<SqlLiteStmt>
SqlLite::insert(const PtrSqlTable& table)
{
    return prepareStatement(table->getInsert());
}


std::unique_ptr<SqlLiteStmt>
SqlLite::select(const PtrSqlTable& table, const std::string& fields, const std::string& where)
{
    std::string select{table->getSelect(fields)};
    if (!where.empty()) {
        select += " WHERE ";
        select += where;
    }
    return prepareStatement(select);
}

PtrSqlLiteStmt
SqlLite::update(const PtrSqlTable& table, const std::vector<PtrSqlColumn>& cols, const std::string& where)
{
    std::string update{table->getUpdate(cols)};
    if (!where.empty()) {
        update += " WHERE ";
        update += where;
    }
    return prepareStatement(update);
}

PtrSqlLiteStmt
SqlLite::deleteFrom(const PtrSqlTable& table, const std::string& where)
{
    std::string update{table->getDeleteFrom()};
    if (!where.empty()) {
        update += " WHERE ";
        update += where;
    }
    return prepareStatement(update);
}


void
SqlLite::execute(const std::string& sql)
{
#   ifdef DEBUG_SQLLITE
    std::cout << "SqlLite::execute " << sql << std::endl;
#   endif
    char* zErrMsg{};
    auto rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        auto msg = std::format("SqlLite::execute error: {} sql {}", zErrMsg, sql);
        sqlite3_free(zErrMsg);
        throw SqlException(msg);
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

PtrSqlLiteStmt
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

PtrSqlLiteStmt
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

PtrSqlLiteStmt
SqlDeleteBuilder::build()
{
    std::string sql("DELETE FROM ");
    sql.reserve(m_where.length() + m_order.length() + 64);
    sql += getTableName();
    sql += buildTail();
    return m_sqlLite->prepareStatement(sql);
}

PtrSqlLiteStmt
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

} // namespace psc::sql
