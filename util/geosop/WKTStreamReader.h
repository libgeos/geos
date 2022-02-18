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

#ifndef GEOSOP_WKTSTREAMREADER_H
#define GEOSOP_WKTSTREAMREADER_H

#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

// Forward declarations
namespace geos {
namespace geom {

class Geometry;
class PrecisionModel;
}
}

class WKTStreamReader {

public:
    WKTStreamReader(std::istream& instr);
    ~WKTStreamReader();

    geos::geom::Geometry* next();

private:

    std::istream& instr;
    geos::io::WKTReader rdr;
};

#endif // GEOSOP_WKTSTREAMREADER_H
