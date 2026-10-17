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
test_sql()
{
    bool ret{};
    try {
        psc::sql::SqlLite sqlLite("db.sqlite");
        psc::sql::PtrSqlTable gn250 = std::make_shared<psc::sql::SqlTable>("gn250");
        gn250->addColumns(std::make_shared<psc::sql::SqlVarcharColumn>("nnid", 16, false,true));
        gn250->addColumns(std::make_shared<psc::sql::SqlVarcharColumn>("name", SQLITE3_TEXT, 255));
        sqlLite.create(gn250);
        auto ins = sqlLite.insert(gn250);
        ins->setString(1, "ghj5");
        ins->setString(2, "Naaaaaaaaame");
        if (ins->exec() == SQLITE_CONSTRAINT) {
            std::cout << "Insert constraint violation!" << std::endl;
        }
        auto ins2 = sqlLite.insert(gn250);
        ins2->setString(1, "ghj6");
        ins2->setString(2, "Naaaaaaaaame");
        if (ins2->exec() == SQLITE_CONSTRAINT) {
            std::cout << "Insert constraint violation!" << std::endl;
        }

        auto stmt = sqlLite.select(gn250, "*", "");
        auto names = stmt->getColumnNames();
        size_t n{};
        while (stmt->next()) {
            if (n == 0) {
                auto types = stmt->getResultTypes();
                for (size_t i = 0; i < names.size(); ++i) {
                    std::cout << "Name " << names[i] << " type " << types[i] << std::endl;
                }
            }
            std::cout << "Row " << names.size() << std::endl;
            for (int i = 0; i < static_cast<int>(names.size()); ++i) {
                auto data = stmt->getString(i);
                std::cout << "    " << names[i] << "=" << data << std::endl;
            }
            ++n;
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

    if (!test_sql()) {
        return 1;
    }
    return 0;
}