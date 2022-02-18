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

#include <geos/io/WKBReader.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <memory> // for unique_ptr
//#include <algorithm>

#include "WKBStreamReader.h"

using namespace geos::geom;

WKBStreamReader::WKBStreamReader(std::istream& instr)
    : instr(instr)
{
}

WKBStreamReader::~WKBStreamReader() {

}

/*public*/


/*
Return: nullptr if at EOF
*/
Geometry*
WKBStreamReader::next()
{
    std::string line;
    std::getline(instr, line);
    if (! instr) {
        return nullptr;
    }
    std::istringstream hex(line);
    auto g = rdr.readHEX( hex );
    return g.release();
}
