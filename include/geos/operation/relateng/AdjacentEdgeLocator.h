/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Location.h>
#include <geos/export.h>


// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
    class NodeSections;
    class NodeSection;
}
}
namespace geom {
    class CoordinateXY;
    class Geometry;
    class LinearRing;
    class Polygon;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng

/**
 * Determines the location for a point which is known to lie
 * on at least one edge of a set of polygons.
 * This provides the union-semantics for determining
 * point location in a GeometryCollection, which may
 * have polygons with adjacent edges which are effectively
 * in the interior of the geometry.
 * Note that it is also possible to have adjacent edges which
 * lie on the boundary of the geometry
 * (e.g. a polygon contained within another polygon with adjacent edges).
 *
 * @author mdavis
 *
 */
class GEOS_DLL AdjacentEdgeLocator {
    using CoordinateXY = geos::geom::CoordinateXY;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using LinearRing = geos::geom::LinearRing;
    using Polygon = geos::geom::Polygon;
    using Location = geos::geom::Location;

public:

    AdjacentEdgeLocator(const Geometry* geom)
    {
        init(geom);
    }

    Location locate(const CoordinateXY* p);

    /**
     * Disable copy construction and assignment. Apparently needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    AdjacentEdgeLocator(const AdjacentEdgeLocator&) = delete;
    AdjacentEdgeLocator& operator=(const AdjacentEdgeLocator&) = delete;


private:

    // Members

    std::vector<const CoordinateSequence*> ringList;

    /*
     * When we have to reorient rings, we end up allocating new
     * rings, since we cannot reorient the rings of the input
     * geometry, so this is where we store those "local" rings.
     */
    std::vector<std::unique_ptr<CoordinateSequence>> localRingList;


    // Methods

    void addSections(
        const CoordinateXY* p,
        const CoordinateSequence* ring,
        NodeSections& sections);

    NodeSection* createSection(
        const CoordinateXY* p,
        const CoordinateXY* prev,
        const CoordinateXY* next);

    void init(const Geometry* geom);

    void addRings(const Geometry* geom);

    void addRing(const LinearRing* ring, bool requireCW);


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

