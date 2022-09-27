/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#include <geos/geom/prep/PreparedPolygonDistance.h>
#include <geos/geom/prep/PreparedPolygonIntersects.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/geom/Geometry.h>

// std
#include <cstddef>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

double
PreparedPolygonDistance::distance(const geom::Geometry* g) const
{
    if ( prepPoly->getGeometry().isEmpty() || g->isEmpty() )
    {
        return DoubleInfinity;
    }

    // If any point from g is contained by prepPoly, the distance is zero
    if ( isAnyTestComponentInTarget(g) ) {
        return 0.0;
    }

    // Perform an indexed distance calculation between the boundaries of prepPoly and g
    operation::distance::IndexedFacetDistance *idf = prepPoly->getIndexedFacetDistance();
    double dist = idf->distance(g);

    // If any point from prepPoly is contained by g, the distance is zero
    // Do this last because this PIP test is not indexed.
    if ( g->getDimension() == 2 && dist > 0 && isAnyTargetComponentInAreaTest(g, prepPoly->getRepresentativePoints())) {
        return 0.0;
    }

    return dist;
}

bool
PreparedPolygonDistance::isWithinDistance(const geom::Geometry* g, double d) const
{
    if ( prepPoly->getGeometry().isEmpty() || g->isEmpty() )
    {
        return false;
    }

    // If any point from g is contained by prepPoly, the distance is zero
    if ( isAnyTestComponentInTarget(g) ) {
        return true;
    }

    // Perform an indexed distance calculation between the boundaries of prepPoly and g
    operation::distance::IndexedFacetDistance *idf = prepPoly->getIndexedFacetDistance();
    bool withinDistance = idf->isWithinDistance(g, d);

    // If any point from prepPoly is contained by g, the distance is zero
    // Do this last because this PIP test is not indexed.
    if ( g->getDimension() == 2 && !withinDistance) {
        return isAnyTargetComponentInAreaTest(g, prepPoly->getRepresentativePoints());
    }

    return withinDistance;
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos
