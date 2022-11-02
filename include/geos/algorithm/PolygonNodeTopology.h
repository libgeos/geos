/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://libgeos.org
 *
 * Copyright (c) 2021 Martin Davis
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>


// Forward declarations
namespace geos {
namespace geom {
class CoordinateXY;
}
}

using geos::geom::CoordinateXY;


namespace geos {
namespace algorithm { // geos::algorithm

/**
 * Functions to compute topological information
 * about nodes (ring intersections) in polygonal geometry.
 *
 * @author mdavis
 *
 */
class GEOS_DLL PolygonNodeTopology {

public:

    /*
    * Check if the segments at a node between two rings (or one ring) cross.
    * The node is topologically valid if the rings do not cross.
    * This function assumes that the segments are not collinear.
    *
    * @param nodePt the node location
    * @param a0 the previous segment endpoint in a ring
    * @param a1 the next segment endpoint in a ring
    * @param b0 the previous segment endpoint in the other ring
    * @param b1 the next segment endpoint in the other ring
    * @return true if the rings cross at the node
    */
    static bool
    isCrossing(const CoordinateXY* nodePt,
        const CoordinateXY* a0, const CoordinateXY* a1,
        const CoordinateXY* b0, const CoordinateXY* b1);


    /**
    * Tests whether an segment node-b lies in the interior or exterior
    * of a corner of a ring formed by the two segments a0-node-a1.
    * The ring interior is assumed to be on the right of the corner
    * (i.e. a CW shell or CCW hole).
    * The test segment must not be collinear with the corner segments.
    *
    * @param nodePt the node location
    * @param a0 the first vertex of the corner
    * @param a1 the second vertex of the corner
    * @param b the other vertex of the test segment
    * @return true if the segment is interior to the ring corner
    */
    static bool isInteriorSegment(const CoordinateXY* nodePt,
        const CoordinateXY* a0, const CoordinateXY* a1, const CoordinateXY* b);


private:

    /**
    * Tests if an edge p is between edges e0 and e1,
    * where the edges all originate at a common origin.
    * The "inside" of e0 and e1 is the arc which does not include the origin.
    * The edges are assumed to be distinct (non-collinear).
    *
    * @param origin the origin
    * @param p the destination point of edge p
    * @param e0 the destination point of edge e0
    * @param e1 the destination point of edge e1
    * @return true if p is between e0 and e1
    */
    static bool isBetween(const CoordinateXY* origin,
        const CoordinateXY* p,
        const CoordinateXY* e0, const CoordinateXY* e1);

    /**
    * Tests if the angle with the origin of a vector P is greater than that of the
    * vector Q.
    *
    * @param origin the origin of the vectors
    * @param p the endpoint of the vector P
    * @param q the endpoint of the vector Q
    * @return true if vector P has angle greater than Q
    */
    static bool isAngleGreater(const CoordinateXY* origin, const CoordinateXY* p, const CoordinateXY* q);

    static int quadrant(const CoordinateXY* origin, const CoordinateXY* p);


};


} // namespace geos::algorithm
} // namespace geos

