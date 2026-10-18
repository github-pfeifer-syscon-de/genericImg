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
#include <map>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <sqlite3.h>

struct sqlite3;
struct sqlite3_stmt;

namespace psc::sql {

class SqlLite;
class SqlLiteStmt;

class SqlException
: public std::runtime_error
{
public:
    SqlException(const std::string& msg);
    virtual ~SqlException() = default;

};

class SqlColumn
{
public:
    SqlColumn(const std::string& name, int type, int size = 0, bool nullable = true, bool primaryKey = false)
    : m_name{name}
    , m_type{type}
    , m_size{size}
    , m_nullable{nullable}
    , m_primaryKey{primaryKey}
    {
    }
    explicit SqlColumn(const SqlColumn& other) = delete;
    virtual ~SqlColumn() = default;

    std::string getName() const {
        return m_name;
    }
    int getType() const {
        return m_type;
    }
    int getSize() const {
        return m_size;
    }
    bool isPrimaryKey() const {
        return m_primaryKey;
    }
    virtual void addCreateSize(std::string& def) const = 0 ;
    std::string getCreate() const {
        std::string def;
        def.reserve(64);
        def += m_name;
        def += ' ';
        switch (m_type) {
        case SQLITE_INTEGER:
            def += "INTEGER";
            break;
        case SQLITE_FLOAT:
            def += "NUMERIC";
            addCreateSize(def);
            break;
        case SQLITE_BLOB:
            def += "BLOB";
            break;
        case SQLITE3_TEXT:
            if (m_size > 255) {
                def += "TEXT";
            }
            else {
                def += "VARCHAR";
                addCreateSize(def);
            }
            break;
        }
        if (m_primaryKey) {
            def += " PRIMARY KEY";
        }
        else if (!m_nullable) {
            def += " NOT NULL";
        }
        return def;
    }
protected:
    std::string m_name;
    int m_type;
    int m_size;
    bool m_nullable;
    bool m_primaryKey;
};

using PtrSqlColumn = std::shared_ptr<SqlColumn>;

class SqlVarcharColumn
: public SqlColumn
{
public:
    SqlVarcharColumn(const std::string& name, int size = 0, bool nullable = true, bool primaryKey = false)
    : SqlColumn(name, SQLITE3_TEXT, size, nullable, primaryKey)
    {
    }
    explicit SqlVarcharColumn(const SqlVarcharColumn& other) = delete;
    virtual ~SqlVarcharColumn() = default;
     void addCreateSize(std::string& def) const override {
        if (m_size > 0) {
            def += '(';
            def += std::to_string(m_size);
            def += ')';
        }
    }

};


class SqlNumColumn
: public SqlColumn
{
public:
    SqlNumColumn(const std::string& name, int size, int decimals = 0, bool nullable = true, bool primaryKey = false)
    : SqlColumn(name, SQLITE_FLOAT, size, nullable, primaryKey)
    , m_decimals{decimals}
    {
    }
    explicit SqlNumColumn(const SqlNumColumn& other) = delete;
    virtual ~SqlNumColumn() = default;

    int getDecimals() const {
        return m_decimals;
    }
    void addCreateSize(std::string& def) const override {
        if (m_size > 0) {
            def += '(';
            def += std::to_string(m_size);
            if (m_decimals > 0) {
                def += ',';
                def += std::to_string(m_decimals);
            }
            def += ')';
        }
    }

private:
    int m_decimals;
};

class SqlTable;

class SqlIndex
{
public:
    SqlIndex(bool unique, const std::string& name, const std::vector<PtrSqlColumn>& columns)
    : m_unique{unique}
    , m_name{name}
    , m_columns{columns}
    {
    }
    SqlIndex(bool unique, const std::string& name, const PtrSqlColumn& column)
    : m_unique{unique}
    , m_name{name}
    {
        m_columns.push_back(column);
    }
    explicit SqlIndex(const SqlIndex& other) = delete;
    virtual ~SqlIndex() = default;

    std::string getCreate(const SqlTable* table) const;
    std::string getName() {
        return m_name;
    }
private:
    bool m_unique;
    std::string m_name;
    std::vector<PtrSqlColumn> m_columns;
};

using PtrSqlIndex = std::shared_ptr<SqlIndex>;

class SqlTable
{
public:
    SqlTable(const std::string& name)
    : m_name{name}
    {
    }
    explicit SqlTable(const SqlTable& other) = delete;
    virtual ~SqlTable() = default;

    // returns bind index
    size_t addColumns(const PtrSqlColumn& column) {
        m_columns.push_back(column);
        return m_columns.size();
    }
    size_t addIndex(const PtrSqlIndex& index) {
        m_indexes.push_back(index);
        return m_indexes.size();
    }
    std::vector<PtrSqlColumn> getColumns() const {
        return m_columns;
    }
    std::string getName() const {
        return m_name;
    }
    std::string getCreate() const;
    std::string getInsert() const;
    std::string getSelect(const std::string& fields) const;
    std::string getUpdate(const std::vector<PtrSqlColumn>& cols) const;
    std::string getDeleteFrom() const;
private:
    std::string m_name;
    std::vector<PtrSqlColumn> m_columns;
    std::vector<PtrSqlIndex> m_indexes;
};

using PtrSqlTable = std::shared_ptr<SqlTable>;

class SqlLiteStmt
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
     *         the see the difference to value "" use getResultTypes
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
{
public:
    SqlLite(const std::string& name);
    explicit SqlLite(const SqlLite& other) = delete;
    virtual ~SqlLite();

    // execute immediate e.g. create table goes here
    void execute(const std::string& stmt);
    // execute where a result is expected
    PtrSqlLiteStmt prepareStatement(const std::string& stmt);
    sqlite3* getDb() {
        return m_db;
    }
    void beginTransaction();
    void commit();
    void rollback();
    void create(const PtrSqlTable& table);
    PtrSqlLiteStmt insert(const PtrSqlTable& table);
    PtrSqlLiteStmt select(const PtrSqlTable& table, const std::string& fields, const std::string& where);
    PtrSqlLiteStmt update(const PtrSqlTable& table, const std::vector<PtrSqlColumn>& cols, const std::string& where);
    PtrSqlLiteStmt deleteFrom(const PtrSqlTable& table, const std::string& where);
private:
    sqlite3* m_db{};
};

// this contains some presumptions:
//   single primary key
class SqlMerge {
public:
    SqlMerge(SqlLite* sqlLite, const std::shared_ptr<SqlTable>& table)
    : m_sqlLite{sqlLite}
    {
        std::vector<PtrSqlColumn> updateCols;
        PtrSqlColumn primaryColumn;
        int32_t n{1};
        for (auto& sqlCol : table->getColumns()) {
            if (sqlCol->isPrimaryKey()) {
                primaryColumn = sqlCol;
                m_insBindIdx = n;
            }
            else {
                updateCols.push_back(sqlCol);
                m_bindIdxMap.insert(std::pair(sqlCol->getName(), n));
                ++n;
            }
        }
        m_updBindIdx = n;
        m_select = sqlLite->select(table, "COUNT(*)", primaryColumn->getName() + "=?");
        m_insert = sqlLite->insert(table);
        m_update = sqlLite->update(table, updateCols, primaryColumn->getName() + "=?");
    }
    void prepare(const std::string& keyValue) {
        m_select->reset();
        m_select->setString(1, keyValue);
        m_cnt = 0;
        if (m_select->next()) {
            m_cnt = m_select->getLong(0);
        }
        if (m_cnt == 0) {
            m_insert->reset();
            m_insert->setString(m_insBindIdx, keyValue);
        }
        else {
            m_update->reset();
            m_update->setString(m_updBindIdx, keyValue);
        }
    }
    void findColumn(const std::string& name, std::function<void(const PtrSqlLiteStmt& stmt, int32_t)> function) {
        auto iter = m_bindIdxMap.find(name);
        if (iter != m_bindIdxMap.end()) {
            auto bindIdx = iter->second;
            if (m_cnt == 0) {
                if (bindIdx >= m_insBindIdx) {
                    ++bindIdx;
                }
                function(m_insert, bindIdx);
            }
            else {
                function(m_update, bindIdx);
            }
        }
    }
    void setString(const std::string& name, const std::string& value) {
        findColumn(name, [&](const PtrSqlLiteStmt& stmt, int32_t n) {
            stmt->setString(n, value);
        });
    }
    void setDouble(const std::string& name, const double value) {
        findColumn(name, [&](const PtrSqlLiteStmt& stmt, int32_t n) {
            stmt->setDouble(n, value);
        });
    }
    int exec() {
        int ret;
        if (m_cnt == 0) {
            ret = m_insert->exec();
        }
        else {
            ret = m_update->exec();
        }
        return ret;
    }
    explicit SqlMerge(const SqlMerge& other) = delete;
    virtual ~SqlMerge() = default;
private:
    SqlLite* m_sqlLite;
    int32_t m_insBindIdx;
    int32_t m_updBindIdx;
    std::map<std::string, int32_t> m_bindIdxMap; // bind indexes for update
    PtrSqlLiteStmt m_select;
    PtrSqlLiteStmt m_insert;
    PtrSqlLiteStmt m_update;
    int64_t m_cnt;
};
} // namespace psc::sql
