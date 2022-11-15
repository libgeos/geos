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

#include <geos/io/WKBStreamReader.h>

using geos::geom::Geometry;

namespace geos {
namespace io {

WKBStreamReader::WKBStreamReader(std::istream& p_instr)
    : instr(p_instr)
{
}

WKBStreamReader::~WKBStreamReader() {

}

/*public*/


/*
Return: nullptr if at EOF
*/
std::unique_ptr<Geometry>
WKBStreamReader::next()
{
    std::string line;
    std::getline(instr, line);
    if (! instr) {
        return nullptr;
    }
    std::istringstream hex(line);
    auto g = rdr.readHEX( hex );
    return g;
}

}
}
