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
#include <geos/io/WKBReader.h>
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

class GEOS_DLL WKBStreamReader {

public:
    WKBStreamReader(std::istream& instr);
    ~WKBStreamReader();

    std::unique_ptr<geom::Geometry> next();

private:

    std::istream& instr;
    WKBReader rdr;
};

}
}


