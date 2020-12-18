/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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

#include <geos/geom/Quadrant.h>

#ifndef GEOS_INLINE
# include <geos/geom/Quadrant.inl>
#endif

namespace geos {
namespace geom { // geos.geom

std::ostream&
operator<<(std::ostream& os, const Quadrant& quadrant)
{
    switch(quadrant) {
        case Quadrant::NE:
            os << "NE";
            break;
        case Quadrant::SE:
            os << "SE";
            break;
        case Quadrant::SW:
            os << "SW";
            break;
        case Quadrant::NW:
            os << "NW";
            break;
    }
    return os;
}

} // namespace geos.geom
} // namespace geos
