/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <vector>

namespace geos {
namespace geom {

/** \brief
 * Utility class for manipulating std::vector<Coordinate>
 */
class GEOS_DLL CoordinateArrays {

public:

    /**
    * Returns whether equals returns true for any two consecutive Coordinates
    * in the given array.
    */
    static bool hasRepeatedPoints(const std::vector<Coordinate>& coords);

    /**
    * Remove any repeated points from the vector, without
    * reallocating a new vector.
    */
    static void removeRepeatedPoints(std::vector<Coordinate>& coords);

    /**
    * Test for the presence of any invalid or repeated points
    */
    static bool hasRepeatedOrInvalid(const std::vector<Coordinate>& coords);

    /**
    * If the coordinate array argument has repeated or invalid points,
    * constructs a new array containing no repeated points.
    * Otherwise, returns the argument.
    * @param coords an array of coordinates
    * @return the array with repeated and invalid coordinates removed
    */
    static void removeRepeatedOrInvalidPoints(std::vector<Coordinate>& coords);

    /**
    * Tests whether an array of {@link Coordinate}s forms a ring,
    * by checking length and closure.
    * Self-intersection is not checked.
    * @param pts an array of Coordinates
    * @return true if the coordinate form a ring.
    */
    static bool isRing(const std::vector<Coordinate>& pts);

};

} // namespace geos::geom
} // namespace geos


