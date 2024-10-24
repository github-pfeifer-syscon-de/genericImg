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

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <format>
#include <Log.hpp>

static bool
test_log()
{
    const std::string logFile{"test.log"};
    std::cout << "Log using " << logFile << std::endl;
    std::remove(logFile.c_str());
    auto log = psc::log::Log::create("test", psc::log::Type::File);
    auto plugin = log->getPlugin();
    auto filePlug = dynamic_pointer_cast<psc::log::FilePlugin>(plugin);
    auto fileLog = Gio::File::create_for_path(logFile);
    filePlug->createLogFile(fileLog);   // dont write default dest on testing
    int n = 15;
    psc::log::Log::logAdd(psc::log::Level::Info, [&] {  // try to get best of two worlds, format checking at compile time + lazy evaluation
                              std::cout << "Log called" << std::endl;
                              return std::format("logged {:#08x}!", n);
                          });
    log->close();   // ensure not "buffered"
    auto fs = std::ifstream(logFile);
    if (fs) {
        std::string buf(80, '\0');
        std::getline(fs, buf);
        fs.close();
        std::cout << "Log buf: " << buf << std::endl;
        auto exp_s{"logged 0x00000f!"};
        auto found = buf.find(exp_s);
        auto exp_n{7u};
        if (buf.length() < exp_n || found == buf.npos)  {
            std::cout << "Log expected at least "
                      << exp_n << " got " << buf.length()
                      << " or not found " << exp_s
                      << std::endl;
            return false;
        }
    }
    else {
        std::cout << "Log file not readable? " << std::endl;
        return false;
    }
    std::remove(logFile.c_str());

    return true;
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Gio::init();   // includes Glib::init
    if (!test_log()) {
        return 1;
    }

    return 0;
}

