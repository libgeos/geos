/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Quadrant.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_GEOM_QUADRANTS_H
#define GEOS_GEOM_QUADRANTS_H

#include <geos/export.h>
#include <geos/inline.h>

#include <geos/geom/Quadrant.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

namespace geos {
namespace geom { // geos.geom

class GEOS_DLL Quadrants {
    /** \brief
     * Utility functions for working with quadrants.
     */
public:
    /**
     * Returns the quadrant of a directed line segment
     * (specified as x and y displacements, which cannot both be 0).
     *
     * @throws IllegalArgumentException if the displacements are both 0
     */
    static Quadrant quadrant(double dx, double dy);

    /**
     * Returns the quadrant of a directed line segment from p0 to p1.
     *
     * @throws IllegalArgumentException if the points are equal
     */
    static Quadrant quadrant(const geom::Coordinate &p0, const geom::Coordinate &p1);

    /**
     * Returns true if the quadrants are 1 and 3, or 2 and 4
     */
    static bool isOpposite(Quadrant quad1, Quadrant quad2);

    /**
     * Returns true if the given quadrant is 0 or 1.
     */
    static bool isNorthern(Quadrant quad);

};

} // namespace geos.geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/Quadrants.inl"
#endif

#endif // ifndef GEOS_GEOM_QUADRANT_H

