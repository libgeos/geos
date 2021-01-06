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

#include "WKTStreamReader.h"

using namespace geos::geom;

WKTStreamReader::WKTStreamReader(std::istream& instr)
    : instr(instr)
{
}

WKTStreamReader::~WKTStreamReader() {

}

/*public*/


/*
Return: nullptr if at EOF
*/
Geometry*
WKTStreamReader::next()
{
    std::string wkt = "";

    int lParen = 0;
    int rParen = 0;
    do {
        std::string line;
        std::getline(instr, line);
        if (! instr) {
            return nullptr;
        }

        lParen += std::count(line.begin(), line.end(), '(');
        rParen += std::count(line.begin(), line.end(), ')');

        wkt += line;
    } while (lParen == 0 || lParen != rParen);

    auto g = rdr.read( wkt.c_str() );
    return g.release();
}
