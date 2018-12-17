/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Orientation.java @ 2017-09-04
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_ORIENTATION_H
#define GEOS_ALGORITHM_ORIENTATION_H

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

namespace geos {
namespace algorithm { // geos::algorithm

/**
* Functions to compute the orientation of points, lines and rings.
*
* @author Martin Davis
*
*/
class GEOS_DLL Orientation {
public:

    /* A value that indicates an orientation or turn */
    enum {
        CLOCKWISE=-1,
        COLLINEAR=0,
        COUNTERCLOCKWISE=1,
        RIGHT=-1,
        LEFT=1,
        STRAIGHT=0
    };

    /*
    * Returns the index of the direction of the point <code>q</code>
    * relative to a vector specified by <code>p1-p2</code>.
    *
    * @param p1 the origin point of the vector
    * @param p2 the final point of the vector
    * @param q the point to compute the direction to
    *
    * @return 1 if q is counter-clockwise (left) from p1-p2
    * @return -1 if q is clockwise (right) from p1-p2
    * @return 0 if q is collinear with p1-p2
    */
    static int index(const geom::Coordinate &p1,
                     const geom::Coordinate &p2, const geom::Coordinate &q);

    /**
    * Computes whether a ring defined by an array of {@link Coordinate}s is
    * oriented counter-clockwise.
    * <ul>
    * <li>The list of points is assumed to have the first and last points equal.
    * <li>This will handle coordinate lists which contain repeated points.
    * </ul>
    * This algorithm is <b>only</b> guaranteed to work with valid rings. If the
    * ring is invalid (e.g. self-crosses or touches), the computed result may not
    * be correct.
    *
    * @param ring
    *          an array of Coordinates forming a ring
    * @return true if the ring is oriented counter-clockwise.
    * @throws IllegalArgumentException
    *           if there are too few points to determine orientation (&lt; 4)
    */
    static bool isCCW(const geom::CoordinateSequence* ring);



};


} // namespace geos::algorithm
} // namespace geos


#endif // GEOS_ALGORITHM_ORIENTATION_H
