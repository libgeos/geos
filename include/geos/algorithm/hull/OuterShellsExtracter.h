/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <vector>

namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class Geometry;
class GeometryCollection;
class GeometryFactory;
class LinearRing;
class Polygon;
}
}

using geos::geom::Geometry;
using geos::geom::LinearRing;

namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull

/**
 * Extracts the rings of outer shells from a polygonal geometry.
 * Outer shells are the shells of polygon elements which
 * are not nested inside holes of other polygons.
 *
 * \author Martin Davis
 */
class OuterShellsExtracter {
private:

    OuterShellsExtracter(const Geometry& g);

    void extractOuterShells(std::vector<const LinearRing*>& outerShells);

    bool isOuter(const LinearRing& shell, std::vector<const LinearRing*>& outerShells);

    bool covers(const LinearRing& shellA, const LinearRing& shellB);

    bool isPointInRing(const LinearRing& shell, const LinearRing& shellRing);

    static void extractShellRings(const Geometry& polygons, std::vector<const LinearRing*>& shells);

    static bool envelopeAreaComparator(
        const LinearRing* g1,
        const LinearRing* g2);

    const Geometry& geom;

public:
    static void extractShells(const Geometry* polygons, std::vector<const LinearRing*>& shells);

};

} // geos::algorithm::hull
} // geos::algorithm
} // geos

