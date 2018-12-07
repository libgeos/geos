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
 **********************************************************************/

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Location.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>

#include <typeinfo>
#include <cassert>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace locate { // geos.algorithm

/**
 * locate is the main location function.  It handles both single-element
 * and multi-element Geometries.  The algorithm for multi-element Geometries
 * is more complex, since it has to take into account the boundaryDetermination rule
 */
int
SimplePointInAreaLocator::locate(const Coordinate& p, const Geometry *geom)
{
    if (geom->isEmpty())
        return Location::EXTERIOR;

    return locateInGeometry(p, geom);
}

int
SimplePointInAreaLocator::locateInGeometry(const Coordinate& p,const Geometry *geom)
{
    if (const Polygon *poly = dynamic_cast<const Polygon*>(geom))
    {
        return locatePointInPolygon(p, poly);
    }

    if (const GeometryCollection *col = dynamic_cast<const GeometryCollection*>(geom))
    {
        for (auto g2: *col)
        {
            assert (g2!=geom);
            int loc = locateInGeometry(p, g2);
            if (loc != Location::EXTERIOR)
                return loc;
        }
    }
    return Location::EXTERIOR;
}

int
SimplePointInAreaLocator::locatePointInPolygon(const Coordinate& p, const Polygon *poly)
{
    if (poly->isEmpty()) return Location::EXTERIOR;
    const LineString *shell=poly->getExteriorRing();
    const CoordinateSequence *cl;
    cl = shell->getCoordinatesRO();
    int shellLoc = CGAlgorithms::locatePointInRing(p,*cl);
    if (shellLoc != Location::INTERIOR)
        return shellLoc;

    // now test if the point lies in or on the holes
    for(size_t i=0, n=poly->getNumInteriorRing(); i<n; i++)
    {
        const LineString *hole = poly->getInteriorRingN(i);
        cl = hole->getCoordinatesRO();
        int holeLoc = CGAlgorithms::locatePointInRing(p,*cl);
        if (holeLoc == Location::BOUNDARY)
            return Location::BOUNDARY;
        if (holeLoc == Location::INTERIOR)
            return Location::EXTERIOR;
        // if in EXTERIOR of this hole, keep checking other holes
    }
    return Location::INTERIOR;
}

} // namespace geos.algorithm.locate
} // namespace geos.algorithm
} // namespace geos
