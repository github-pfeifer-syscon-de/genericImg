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

#include <memory>
#include <unordered_map>
#include <functional>

namespace psc::sql {

enum class Datatype {
    None
    , Integer
    , Numeric
    , Text  /* depending on size > 255 this may be varchar or text */
    , Blob
    ,
};


enum class DbType {
    None
    , Sqlite
    , Mysql
    , Postgres
    ,
};

class SqlException
: public std::runtime_error
{
public:
    SqlException(const std::string& msg);
    virtual ~SqlException() = default;

};

class SqlTable;
using PtrSqlTable = std::shared_ptr<SqlTable>;

class SqlStatement
{
public:
    SqlStatement() = default;
    explicit SqlStatement(const SqlStatement& other) = delete;
    virtual ~SqlStatement() = default;

    virtual void reset() = 0;   // make stmt ready for next bind
    virtual bool next() = 0;    // this is intended for selects
    virtual int exec() = 0;
    virtual int64_t getRowCount() = 0;
    virtual std::vector<std::string> getColumnNames() = 0;
    virtual std::vector<int> getResultTypes() = 0;
    virtual std::string getString(int col) = 0;
    virtual double getDouble(int col) = 0;
    virtual int64_t getLong(int col) = 0;
    virtual std::string getString(const std::string& colName) = 0;
    virtual double getDouble(const std::string& colName) = 0;
    virtual int64_t getLong(const std::string& colName) = 0;
    virtual void setString(int bindIdx, const std::string& value) = 0;
    virtual void setDouble(int bindIdx, double value) = 0;
    virtual void setLong(int bindIdx, int64_t value) = 0;
    virtual void setNull(int bindIdx) = 0;

};

using PtrSqlStatement = std::unique_ptr<SqlStatement>;
class Metadata;
using PtrMetadata = std::unique_ptr<Metadata>;
class SqlSelectBuilder;
using PtrSqlSelectBuilder = std::unique_ptr<SqlSelectBuilder>;
class SqlUpdateBuilder;
using PtrSqlUpdateBuilder = std::unique_ptr<SqlUpdateBuilder>;
class SqlDeleteBuilder;
using PtrSqlDeleteBuilder = std::unique_ptr<SqlDeleteBuilder>;
class SqlInsertBuilder;
using PtrSqlInsertBuilder = std::unique_ptr<SqlInsertBuilder>;

class Database
{
public:
    Database() = default;
    explicit Database(const Database& other) = delete;
    virtual ~Database() = default;

    static std::shared_ptr<Database> create(DbType dbType, const std::string& name);
    virtual void execute(const std::string& stmt) = 0;
    // execute where a result is expected
    virtual PtrSqlStatement prepareStatement(const std::string& stmt) = 0;
    virtual void beginTransaction() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    virtual void create(const PtrSqlTable& table) = 0;
    virtual PtrMetadata getMetadata() = 0;
    virtual PtrSqlSelectBuilder buildSelect(const PtrSqlTable& table);
    virtual PtrSqlSelectBuilder buildSelect(const std::string& tableNanme);
    virtual PtrSqlInsertBuilder buildInsert(const PtrSqlTable& table);
    virtual PtrSqlInsertBuilder buildInsert(const std::string& tableName);
    virtual PtrSqlUpdateBuilder buildUpdate(const PtrSqlTable& table);
    virtual PtrSqlUpdateBuilder buildUpdate(const std::string& tableName);
    virtual PtrSqlDeleteBuilder buildDelete(const PtrSqlTable& table);
    virtual PtrSqlDeleteBuilder buildDelete(const std::string& tableName);

};

using PtrDatabase = std::shared_ptr<Database>;

class SqlColumn
{
public:
    SqlColumn(const std::string& name, Datatype type, int size = 0, bool nullable = true, bool primaryKey = false)
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
    Datatype getType() const {
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
        case Datatype::Integer:
            def += "INTEGER";
            break;
        case Datatype::Numeric:
            def += "NUMERIC";
            addCreateSize(def);
            break;
        case Datatype::Blob:
            def += "BLOB";
            break;
        case Datatype::Text:
            if (m_size > 255) {
                def += "TEXT";
            }
            else {
                def += "VARCHAR";
                addCreateSize(def);
            }
            break;
        default:
            def += "ANY";
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
    Datatype m_type;
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
    : SqlColumn(name, Datatype::Text, size, nullable, primaryKey)
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
    : SqlColumn(name, size > 8 || decimals > 0 ? Datatype::Numeric : Datatype::Integer, size, nullable, primaryKey)
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

class SqlBuilder
{
public:
    virtual ~SqlBuilder() = default;
    void setTable(const PtrSqlTable& table) {
        m_table = table;
    }
    void setTableName(const std::string& tableName) {
        m_tableName = tableName;
    }
protected:
    SqlBuilder(Database* db)
    : m_db{db}
    {
    }
    virtual PtrSqlStatement build() = 0;
    std::string getTableName() {
        if (!m_tableName.empty()) {
            return m_tableName;
        }
        return m_table->getName();
    }
    Database* m_db;
    PtrSqlTable m_table;
    std::string m_tableName;
};

enum class OrderBy {
    None
    ,Ascending
    ,Descending
    ,
};

enum class JoinBy {
    And
    ,Or
    ,
};


class SqlWhereBuilder
: public SqlBuilder
{
public:
    virtual ~SqlWhereBuilder() = default;

    SqlWhereBuilder* addWhere(const std::string where) {
	if (!m_where.empty()) {
	    m_where += getJoinExpr();
	}
	m_where += where;
        return this;
    }
    SqlWhereBuilder* addOrderBy(const std::string order, OrderBy orderBy = OrderBy::None);
    SqlWhereBuilder* setJoin(JoinBy join) {
        m_join = join;
        return this;
    }
protected:
    SqlWhereBuilder(Database* db)
    : SqlBuilder(db)
    {
    }
    std::string getJoinExpr();
    std::string buildTail();
    std::string m_where;
    std::string m_order;
    JoinBy m_join;
};

class SqlSelectBuilder
: public SqlWhereBuilder
{
public:
    SqlSelectBuilder(Database* db)
    : SqlWhereBuilder(db)
    {
    }
    virtual ~SqlSelectBuilder() = default;
    SqlSelectBuilder* addSelect(const std::string& select) {
        if (!m_select.empty()) {
            m_select += ',';
        }
        m_select += select;
        return this;
    }
    PtrSqlStatement build() override;
protected:
    std::string m_select;
};

class SqlUpdateBuilder
: public SqlWhereBuilder
{
public:
    SqlUpdateBuilder(Database* db)
    : SqlWhereBuilder(db)
    {
    }
    virtual ~SqlUpdateBuilder() = default;
    SqlUpdateBuilder* addUpdate(const std::string& update, bool useBind = true) {
        if (!m_update.empty()) {
            m_update += ',';
        }
        m_update += update;
        if (useBind) {
            m_update += "=?";
        }
        return this;
    }
    PtrSqlStatement build() override;
protected:
    std::string m_update;
};

class SqlDeleteBuilder
: public SqlWhereBuilder
{
public:
    SqlDeleteBuilder(Database* db)
    : SqlWhereBuilder(db)
    {
    }
    virtual ~SqlDeleteBuilder() = default;
    PtrSqlStatement build() override;
protected:
};

class SqlInsertBuilder
: public SqlBuilder
{
public:
    SqlInsertBuilder(Database* db)
    : SqlBuilder(db)
    {
    }
    virtual ~SqlInsertBuilder() = default;
    SqlInsertBuilder* addInsert(const std::string& insertColumn) {
        if (!m_insert.empty()) {
            m_insert += ',';
        }
        m_insert += insertColumn;
        ++m_insertCount;
        return this;
    }
    PtrSqlStatement build() override;
protected:
    std::string m_insert;
    int m_insertCount{};
};


// this contains some presumptions:
//   single primary key
class SqlMerge {
public:
    SqlMerge(const PtrDatabase& sqlLite, const std::shared_ptr<SqlTable>& table)
    : m_sqlLite{sqlLite}
    {
        PtrSqlColumn primaryColumn;
        int32_t n{1};
        auto updBld = sqlLite->buildUpdate(table);
        for (auto& sqlCol : table->getColumns()) {
            if (sqlCol->isPrimaryKey()) {
                primaryColumn = sqlCol;
                m_insBindIdx = n;
            }
            else {
                updBld->addUpdate(sqlCol->getName());
                m_bindIdxMap.insert(std::pair(sqlCol->getName(), n));
                ++n;
            }
        }
        m_updBindIdx = n;
        auto selBld = sqlLite->buildSelect(table);
        selBld->addSelect("COUNT(*)")
            ->addWhere(primaryColumn->getName() + "=?");
        m_select = selBld->build();
        auto insBld = sqlLite->buildInsert(table);
        m_insert = insBld->build();
        updBld->addWhere(primaryColumn->getName() + "=?");
        m_update = updBld->build();
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
    void findColumn(const std::string& name, std::function<void(const PtrSqlStatement& stmt, int32_t)> function) {
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
        findColumn(name, [&](const PtrSqlStatement& stmt, int32_t n) {
            stmt->setString(n, value);
        });
    }
    void setDouble(const std::string& name, const double value) {
        findColumn(name, [&](const PtrSqlStatement& stmt, int32_t n) {
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
    PtrDatabase m_sqlLite;
    int32_t m_insBindIdx;
    int32_t m_updBindIdx;
    std::unordered_map<std::string, int32_t> m_bindIdxMap; // bind indexes for update
    PtrSqlStatement m_select;
    PtrSqlStatement m_insert;
    PtrSqlStatement m_update;
    int64_t m_cnt;
};

class Metadata {
public:
    virtual ~Metadata() = default;

    virtual PtrSqlStatement getTableList() = 0;
    virtual PtrSqlStatement getObjectList(const std::string& tableName) = 0;
    virtual PtrSqlStatement getObjectDetail(const std::string& type, const std::string& objectName) = 0;
};

} /* namespace psc::sql */