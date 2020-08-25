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
    auto gf = GeometryFactory::create(pm, geom->getSRID());
    // OverlayNG(const geom::Geometry* geom0, const geom::Geometry* geom1, const geom::GeometryFactory* p_geomFact, int p_opCode)
    OverlayNG ov(geom, nullptr, gf.get(), OverlayNG::UNION);
    return ov.getResult();
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
