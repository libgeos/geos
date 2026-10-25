/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Jeroen Bloemscheer
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

namespace geos {
namespace geom {

/// An ST_AffinePlacement (ISO/IEC 13249-3): a location and the images of
/// the two axes of a local 2D coordinate system.
struct GEOS_DLL AffinePlacement {
    Coordinate origin;
    Coordinate u;
    Coordinate v;
};

}
}
