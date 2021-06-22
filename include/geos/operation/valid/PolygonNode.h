/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>


#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}
namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Coordinate;

class GEOS_DLL PolygonNode {

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
    static bool
    isBetween(const Coordinate* origin, const Coordinate* p,
        const Coordinate* e0, const Coordinate* e1);

    /**
    * Tests if the angle with the origin of a vector P is greater than that of the
    * vector Q.
    *
    * @param origin the origin of the vectors
    * @param p the endpoint of the vector P
    * @param q the endpoint of the vector Q
    * @return true if vector P has angle greater than Q
    */
    static bool
    isAngleGreater(const Coordinate* origin,
        const Coordinate* p, const Coordinate* q);

    static int
    quadrant(const Coordinate* origin, const Coordinate* p);


public:

    /**
    * Check if the edges at a node between two rings (or one ring) cross.
    * The node is topologically valid if the ring edges do not cross.
    * This function assumes that the edges are not collinear.
    *
    * @param nodePt the node location
    * @param a0 the previous edge endpoint in a ring
    * @param a1 the next edge endpoint in a ring
    * @param b0 the previous edge endpoint in the other ring
    * @param b1 the next edge endpoint in the other ring
    * @return true if the edges cross at the node
    */
    static bool isCrossing(const Coordinate* nodePt,
        const Coordinate* a0, const Coordinate* a1,
        const Coordinate* b0, const Coordinate* b1);

    /**
    * Tests whether an edge node-b lies in the interior or exterior
    * of a corner of a ring given by a0-node-a1.
    * The ring interior is assumed to be on the right of the corner (a CW ring).
    * The edge must not be collinear with the corner segments.
    *
    * @param nodePt the node location
    * @param a0 the first vertex of the corner
    * @param a1 the second vertex of the corner
    * @param b the destination vertex of the edge
    * @return true if the edge is interior to the ring corner
    */
    static bool isInteriorSegment(const Coordinate* nodePt,
        const Coordinate* a0, const Coordinate* a1,
        const Coordinate* b);

};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

