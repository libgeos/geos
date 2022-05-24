/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <queue>
#include <deque>
#include <map>

namespace geos {
namespace geom {
class Coordinate;
class Envelope;
class Geometry;
class GeometryCollection;
class GeometryFactory;
class LinearRing;
class MultiPolygon;
class Polygon;
}
}

using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::MultiPolygon;
using geos::geom::Polygon;


namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull


/**
 * Constructs a concave hull of a set of polygons, respecting
 * the polygons as constraints.
 * A concave hull is a possibly non-convex polygon containing all the input polygons.
 * A given set of polygons has a sequence of hulls of increasing concaveness,
 * determined by a numeric target parameter.
 * The computed hull "fills the gap" between the polygons,
 * and does not intersect their interior.
 *
 * The concave hull is constructed by removing the longest outer edges
 * of the Delaunay Triangulation of the space between the polygons,
 * until the target criterion parameter is reached.
 *
 * The target criteria are:
 *  * Maximum Edge Length - the length of the longest edge between the polygons is no larger
 *    than this value.
 *  * Maximum Edge Length Ratio - determine the Maximum Edge Length
 *    as a fraction of the difference between the longest and shortest edge lengths
 *    between the polygons. This normalizes the Maximum Edge Length to be scale-free.
 *    A value of 1 produces the convex hull; a value of 0 produces the original polygons.
 *
 * The preferred criterion is the Maximum Edge Length Ratio, since it is
 * scale-free and local (so that no assumption needs to be made about the
 * total amount of concaveness present).
 *
 * Optionally the concave hull can be allowed to contain holes, via setHolesAllowed().
 *
 * The hull can be specified as being "tight", which means it follows the outer boundaries
 * of the input polygons.
 *
 * The input polygons must form a valid MultiPolygon
 * (i.e. they must be non-overlapping).
 *
 * \author Martin Davis
 *
 */
class GEOS_DLL ConcaveHullOfPolygons {

private:

    static constexpr int FRAME_EXPAND_FACTOR = 4;

    Geometry inputPolygons;
    double maxEdgeLength = -1;
    double maxEdgeLengthRatio = -1;
    bool isHolesAllowed = false;
    bool isTight = false;

    GeometryFactory geomFactory;
    LinearRing[] polygonRings;

    Set<Tri> hullTris;
    ArrayDeque<Tri> borderTriQue;
      /**
       * Records the edge index of the longest border edge for border tris,
       * so it can be tested for length and possible removal.
       */
    Map<Tri, Integer> borderEdgeMap = new HashMap<Tri, Integer>();


public:



};



} // geos::algorithm::hull
} // geos::algorithm
} // geos

