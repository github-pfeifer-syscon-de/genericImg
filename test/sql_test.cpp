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

#include <Sqlite.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

static bool
test_sql(const std::shared_ptr<psc::sql::SqlLite>& sqlLite, const psc::sql::PtrSqlTable& gn250)
{
    bool ret{};
    try {
        auto ins = sqlLite->insert(gn250);
        ins->setString(1, "ghj5");
        ins->setString(2, "Naaaaaaaaame");
        ins->setDouble(3, 3.14);
        if (ins->exec() == SQLITE_CONSTRAINT) {
            std::cout << "Insert constraint violation!" << std::endl;
        }
        ins->setString(1, "ghj6");
        ins->setString(2, "Naaaaaaaaame");
        ins->setDouble(3, 6.28);
        if (ins->exec() == SQLITE_CONSTRAINT) {
            std::cout << "Insert constraint violation!" << std::endl;
        }
        auto stmt = sqlLite->select(gn250, "*", "");
        auto names = stmt->getColumnNames();
        size_t n{};
        bool complete{true};
        while (stmt->next()) {
            auto types = stmt->getResultTypes();
            //for (size_t i = 0; i < names.size(); ++i) {
            //    std::cout << "Name " << names[i] << " type " << types[i] << std::endl;
            //}
            std::cout << "Row with " << names.size() << " columns" << std::endl;
            for (int i = 0; i < static_cast<int>(names.size()); ++i) {
                if (types[i] == psc::sql::Datatype::Text) {
                    auto data = stmt->getString(i);
                    std::cout << "    " << names[i] << "=\"" << data << "\"" << std::endl;
                    if (i == 0 && ((n == 0 && data != "ghj5") || (n == 1 && data != "ghj6"))) {
                        complete = false;
                    }
                    if (i == 1 && data != "Naaaaaaaaame") {
                        complete = false;
                    }
                }
                else {
                    auto data = stmt->getDouble(i);
                    std::cout << "    " << names[i] << "=" << data << "" << std::endl;
                    if ((n == 0 && data != 3.14) || (n == 1 && data != 6.28)) {
                        complete = false;
                    }
                }
            }
            ++n;
        }
        ret = complete && n == 2;
    }
    catch (const std::exception& err) {
        std::cout << "Error " << err.what() << std::endl;
    }
    return ret;
}

static bool
test_build(const std::shared_ptr<psc::sql::SqlLite>& sqlLite, const psc::sql::PtrSqlTable& gn250)
{
    bool ret = false;
    try {
        auto upd = sqlLite->buildUpdate(gn250);
        upd->addUpdate("name")
            ->addWhere("nnid=?");
        auto upStmt = upd->build();
        upStmt->setString(1, "Name");
        upStmt->setString(2, "ghj6");
        upStmt->exec();
        auto sel = sqlLite->buildSelect(gn250);
        sel->addSelect("count(*)");
        sel->addWhere("name='Name'");
        auto selStmt = sel->build();
        int64_t n{};
        if (selStmt->next()) {
            n = selStmt->getLong(0);
        }
        if (n != 1) {
            std::cout << "test_build (1) expected count 1 got " << n << std::endl;
            return false;
        }
        auto del = sqlLite->buildDelete(gn250);
        auto delStmt = del->build();
        delStmt->exec();
        auto ins = sqlLite->buildInsert(gn250);
        auto insStmt = ins->build();
        insStmt->setString(1, "ghj7");
        insStmt->setString(2, "Name");
        insStmt->setLong(3, 67);
        insStmt->exec();
        auto selStmt2 = sel->build();
        n = {};
        if (selStmt2->next()) {
            n = selStmt2->getLong(0);
        }
        if (n != 1) {
            std::cout << "test_build (2) expected count 1 got " << n << std::endl;
            return false;
        }
        auto meta = sqlLite->getMetadata();
        auto stmtTbl = meta->getTableList();
        bool tableFound = false;
        bool columnFound = false;
        while (stmtTbl->next()) {
            tableFound = true;
            auto tableName = stmtTbl->getString("name");
            auto tableType = stmtTbl->getString("type");
            std::cout << tableType << " " << tableName << std::endl;
            auto stmtDetail = meta->getObjectDetail("table", tableName);
            while (stmtDetail->next()) {
                columnFound = true;
                auto detailName = stmtDetail->getString("name");
                auto detailType = stmtDetail->getString("type");
                std::cout << "   " << detailName << " " << detailType << std::endl;
            }
            std::cout << "-------" << std::endl;
            auto stmtObj = meta->getObjectList(tableName);
            while (stmtObj->next()) {
                auto objType = stmtObj->getString("type");
                auto objName = stmtObj->getString("name");
                std::cout << "   " << objType << " " << objName  << std::endl;
                if (objType == "index") {
                    auto stmtDetail = meta->getObjectDetail(objType, objName);
                    while (stmtDetail->next()) {
                        auto detailName = stmtDetail->getString("name");
                        std::cout << "      " << detailName << std::endl;
                    }
                }
            }
        }
        if (!columnFound) {
            std::cout << "test_build (3) expected column info not found!" << n << std::endl;
            return false;
        }
        if (!tableFound) {
            std::cout << "test_build (3) expected table info not found!" << n << std::endl;
            return false;
        }
        ret = true;
    }
    catch (const std::exception& err) {
        std::cout << "Error " << err.what() << std::endl;
    }
    return ret;
}

int main(int argc, char** argv)
{
    //std::setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    //Glib::init();
    std::filesystem::path db("db.sqlite");
    if (std::filesystem::exists(db)) {
        std::filesystem::remove(db);
    }
    auto sqlLite =std::make_shared<psc::sql::SqlLite>(db.string());
    psc::sql::PtrSqlTable gn250 = std::make_shared<psc::sql::SqlTable>("gn250");
    gn250->addColumns(std::make_shared<psc::sql::SqlVarcharColumn>("nnid", 16, false,true));
    gn250->addColumns(std::make_shared<psc::sql::SqlVarcharColumn>("name", 32));
    gn250->addColumns(std::make_shared<psc::sql::SqlNumColumn>("num", 10, 5));
    sqlLite->create(gn250);

    if (!test_sql(sqlLite, gn250)) {
        return 1;
    }
    if (!test_build(sqlLite, gn250)) {
        return 2;
    }
    return 0;
}