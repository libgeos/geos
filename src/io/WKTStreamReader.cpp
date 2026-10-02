/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/io/WKTReader.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <memory> // for unique_ptr
#include <algorithm>

#include <geos/io/WKTStreamReader.h>

using namespace geos::geom;

namespace geos {
namespace io {

static bool
isWhitespaceOnly(const std::string& s)
{
    return s.find_last_not_of(" \t\r\n") == std::string::npos;
}

static bool
isEndedWithEmpty(const std::string& s)
{
    auto pos = s.find_last_not_of(" \t\r\n");
    if (pos == std::string::npos || pos < 4) {
        return false;
    }
    std::string tail = s.substr(pos - 4, 5);
    for (char& c : tail) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    if (tail == "EMPTY") {
        if (pos == 4 || std::isspace(static_cast<unsigned char>(s[pos - 5]))) {
            return true;
        }
    }
    return false;
}

WKTStreamReader::WKTStreamReader(std::istream& p_instr)
    : instr(p_instr)
{
}

WKTStreamReader::~WKTStreamReader() {

}

/*public*/


/*
Return: nullptr if at EOF
*/
std::unique_ptr<Geometry>
WKTStreamReader::next()
{
    std::string wkt = "";

    std::string::difference_type lParen = 0;
    std::string::difference_type rParen = 0;
    while (true) {
        std::string line;
        if (!std::getline(instr, line)) {
            if (isWhitespaceOnly(wkt)) {
                return nullptr;
            }
            break;
        }

        lParen += std::count(line.begin(), line.end(), '(');
        rParen += std::count(line.begin(), line.end(), ')');

        if (!wkt.empty()) {
            wkt += "\n";
        }
        wkt += line;

        if (lParen == 0 && rParen == 0 && isWhitespaceOnly(wkt)) {
            wkt.clear();
            continue;
        }

        if ((lParen > 0 && lParen == rParen) ||
            (lParen == 0 && rParen == 0 && isEndedWithEmpty(wkt))) {
            break;
        }
    }

    auto g = rdr.read( wkt.c_str() );
    return g;
}


}
}
