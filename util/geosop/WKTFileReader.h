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

#ifndef GEOSOP_WKTFILEREADER_H
#define GEOSOP_WKTFILEREADER_H

#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

// Forward declarations
namespace geos {
namespace geom {

class Geometry;
class PrecisionModel;
}
}

class WKTFileReader {

public:
    WKTFileReader();
    ~WKTFileReader();

    std::vector<geos::geom::Geometry*> read(std::string fname);

private:
    geos::geom::Geometry* readGeom(std::ifstream& f, geos::io::WKTReader& rdr);
};

#endif // GEOSOP_WKTFILEREADER_H
