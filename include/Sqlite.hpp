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
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <sqlite3.h>

#include "Database.hpp"

struct sqlite3;
struct sqlite3_stmt;

namespace psc::sql {

class SqlLite;
class SqlLiteStmt;


class SqlLiteStmt
: public SqlStatement
{
public:
    SqlLiteStmt(SqlLite* sqlLite, const std::string& sql);
    explicit SqlLiteStmt(const SqlLiteStmt& other) = delete;
    virtual ~SqlLiteStmt();

    void reset();   // make stmt ready for next bind
    bool next();    // this is intended for selects
    // this is intended for dml e.g. insert
    //   @return same as sqlite e.g. constrain violated for existing data e.g. SQLITE_CONSTRAINT
    //   @Exception for probably "unfixable" errors from client side
    int exec();
    int64_t getRowCount();
    std::vector<std::string> getColumnNames();
    std::vector<int> getResultTypes();
    /**
     * @param col the column index
     * @return on best effort if null "" will be returned,
     *         to see the difference to value "" use getResultTypes
     */
    std::string getString(int col);
    double getDouble(int col);
    int64_t getLong(int col);
    std::string getString(const std::string& colName);
    double getDouble(const std::string& colName);
    int64_t getLong(const std::string& colName);
    void setString(int bindIdx, const std::string& value);
    void setDouble(int bindIdx, double value);
    void setLong(int bindIdx, int64_t value);
    void setNull(int bindIdx);

protected:
    int getColumnIdx(const std::string& name);

private:
    SqlLite* m_sqlLite;
    std::string m_sql;
    sqlite3_stmt* m_stmt{};
    std::unordered_map<std::string, int32_t> m_nameCols;
};

using PtrSqlLiteStmt = std::unique_ptr<SqlLiteStmt>;

class SqlLite
: public Database
{
public:
    SqlLite(const std::string& name);
    explicit SqlLite(const SqlLite& other) = delete;
    virtual ~SqlLite();

    // execute immediate e.g. create table goes here
    void execute(const std::string& stmt);
    // execute where a result is expected
    PtrSqlStatement prepareStatement(const std::string& stmt);
    sqlite3* getDb() {
        return m_db;
    }
    void beginTransaction();
    void commit();
    void rollback();
    void create(const PtrSqlTable& table);
    PtrSqlStatement insert(const PtrSqlTable& table);
    PtrSqlStatement select(const PtrSqlTable& table, const std::string& fields, const std::string& where);
    PtrSqlStatement update(const PtrSqlTable& table, const std::vector<PtrSqlColumn>& cols, const std::string& where);
    PtrSqlStatement deleteFrom(const PtrSqlTable& table, const std::string& where);
    PtrMetadata getMetadata() override;
private:
    sqlite3* m_db{};
};

using PtrSqlLite = std::shared_ptr<SqlLite>;

class SqliteMeta
: public Metadata
{
public:
    SqliteMeta(SqlLite* sqlite);
    explicit SqliteMeta(const SqliteMeta& other) = delete;
    virtual ~SqliteMeta() = default;

    // get tables, views
    PtrSqlStatement getTableList() override;
    // get object e.g. index infos related to tableName
    PtrSqlStatement getObjectList(const std::string& tableName) override;
    // get details like columns for the type and name
    PtrSqlStatement getObjectDetail(const std::string& type, const std::string& objectName) override;

protected:
    SqlLite* m_sqlite;
};

} // namespace psc::sql
