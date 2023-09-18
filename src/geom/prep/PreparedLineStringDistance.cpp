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


#include <geos/geom/prep/PreparedLineString.h>
#include <geos/geom/prep/PreparedLineStringDistance.h>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

double
PreparedLineStringDistance::distance(const geom::Geometry* g) const
{
    if ( prepLine.getGeometry().isEmpty() || g->isEmpty() )
    {
        return DoubleInfinity;
    }

    /* Compute potential distance from facets */
    operation::distance::IndexedFacetDistance *idf = prepLine.getIndexedFacetDistance();
    double dist = idf->distance(g);
    if (dist == 0.0)
        return 0.0;

    // If any point from prepLine is contained by g, the distance is zero
    // Do this last because this PIP test is not indexed.
    if ( g->getDimension() == 2 
            && prepLine.isAnyTargetComponentInTest(g)) {
        return 0.0;
    }
    return dist;
}

bool
PreparedLineStringDistance::isWithinDistance(const geom::Geometry* g, double d) const
{
    if ( prepLine.getGeometry().isEmpty() || g->isEmpty() )
    {
        return false;
    }

    operation::distance::IndexedFacetDistance *idf = prepLine.getIndexedFacetDistance();
    if (idf->isWithinDistance(g, d))
        return true;

    // If any point from prepLine is contained by g, the distance is zero
    // Do this last because this PIP test is not indexed.
    if ( g->getDimension() == 2 ) {
        return prepLine.isAnyTargetComponentInTest(g);
    }
    return false;
}


} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos
