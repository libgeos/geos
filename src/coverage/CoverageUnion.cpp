/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageUnion.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/overlayng/CoverageUnion.h>

using geos::geom::Geometry;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public static */
std::unique_ptr<Geometry>
CoverageUnion::Union(std::vector<const Geometry*>& coverage)
{
    if (coverage.size() == 0)
        return nullptr;

    const GeometryFactory* geomFact = coverage[0]->getFactory();
    std::unique_ptr<GeometryCollection> geoms(geomFact->createGeometryCollection(coverage));
    return operation::overlayng::CoverageUnion::geomunion(geoms.get());
}


} // namespace geos.coverage
} // namespace geos


