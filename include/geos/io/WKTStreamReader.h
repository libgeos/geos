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

#pragma once

#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>
#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {

class Geometry;
class PrecisionModel;
}
}

namespace geos {
namespace io {

class GEOS_DLL WKTStreamReader {

public:
    WKTStreamReader(std::istream& instr);
    ~WKTStreamReader();

    std::unique_ptr<geos::geom::Geometry> next();

private:

    std::istream& instr;
    WKTReader rdr;
};

}
}
