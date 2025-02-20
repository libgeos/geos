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

#include <geos/io/WKTFileReader.h>

using namespace geos::geom;

namespace geos {
namespace io {

WKTFileReader::WKTFileReader()
{

}

WKTFileReader::~WKTFileReader() {

}

/*public*/
std::vector<std::unique_ptr<Geometry>>
WKTFileReader::read(std::string fname)
{
    std::ifstream f( fname );
    std::vector<std::unique_ptr<Geometry>> geoms;
    geos::io::WKTReader rdr;

    while (true) {
        auto g = readGeom( f, rdr );
        if (g == nullptr) {
            break;
        }
        geoms.push_back(std::move(g));
    }
    f.close();

    return geoms;
}

/*
Return: nullptr if at EOF
*/
std::unique_ptr<Geometry>
WKTFileReader::readGeom(std::ifstream& f, geos::io::WKTReader& rdr)
{
    std::string wkt = "";

    std::string::difference_type lParen = 0;
    std::string::difference_type rParen = 0;
    do {
        std::string line;
        std::getline(f, line);
        if (! f) {
            return nullptr;
        }

        lParen += std::count(line.begin(), line.end(), '(');
        rParen += std::count(line.begin(), line.end(), ')');

        wkt += line;
    } while (lParen == 0 || lParen != rParen);

    auto g = rdr.read( wkt.c_str() );
    return g;
}

}
}
