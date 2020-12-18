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

#ifndef GEOS_GEOM_QUADRANT_H
#define GEOS_GEOM_QUADRANT_H

#include <geos/export.h>
#include <ostream>

namespace geos {
namespace geom { // geos.geom

enum class GEOS_DLL Quadrant : char {
    NE = 0,
    NW = 1,
    SW = 2,
    SE = 3
};

GEOS_DLL std::ostream& operator<<(std::ostream& os, const Quadrant& quadrant);

} // namespace geos.geom
} // namespace geos

#endif // ifndef GEOS_GEOM_QUADRANT_H

