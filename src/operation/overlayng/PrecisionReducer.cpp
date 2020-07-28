/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/PrecisionReducer.h>

#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/geom/Geometry.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public static*/
std::unique_ptr<Geometry>
PrecisionReducer::reducePrecision(const Geometry* geom, const PrecisionModel* pm)
{
    std::unique_ptr<Geometry> reduced = OverlayNG::geomunion(geom, pm);
    return reduced;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
