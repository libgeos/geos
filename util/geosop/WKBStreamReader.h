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

#ifndef GEOSOP_WKBSTREAMREADER_H
#define GEOSOP_WKBSTREAMREADER_H

#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKBReader.h>

// Forward declarations
namespace geos {
namespace geom {

class Geometry;
class PrecisionModel;
}
}

class WKBStreamReader {

public:
    WKBStreamReader(std::istream& instr);
    ~WKBStreamReader();

    geos::geom::Geometry* next();

private:

    std::istream& instr;
    geos::io::WKBReader rdr;
};

#endif // GEOSOP_WKBSTREAMREADER_H
