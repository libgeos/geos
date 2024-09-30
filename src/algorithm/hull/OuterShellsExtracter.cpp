
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

#include <geos/algorithm/hull/OuterShellsExtracter.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>

using geos::algorithm::PointLocation;
using geos::geom::Polygon;
using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {
namespace hull {

/* public static */
void 
OuterShellsExtracter::extractShells(const Geometry* polygons, std::vector<const LinearRing*>& shells)
{
    OuterShellsExtracter extracter(*polygons);
    extracter.extractOuterShells(shells);
}

/* private */
OuterShellsExtracter::OuterShellsExtracter(const geom::Geometry& g)
    : geom(g)
{
}

/* private */
void 
OuterShellsExtracter::extractOuterShells(std::vector<const LinearRing*>& outerShells) 
{
    std::vector<const LinearRing*> shells;
    extractShellRings(geom, shells);
    //-- sort shells in order of increasing envelope area
    std::sort(shells.begin(), shells.end(), envelopeAreaComparator);

    //-- Scan shells by decreasing area to ensure that shells are added before any nested shells
    for (auto i = shells.rbegin(); i != shells.rend(); ++i) { 
      const LinearRing* shell = *i;
      if (outerShells.size() == 0 
          || isOuter(*shell, outerShells)) {
        outerShells.push_back(shell);
      }
    }
}

bool
OuterShellsExtracter::envelopeAreaComparator(
        const LinearRing* g1,
        const LinearRing* g2)
{
    double area1 = g1->getEnvelopeInternal()->getArea();
    double area2 = g2->getEnvelopeInternal()->getArea();
    if (area1 < area2)
        return true;
    else
        return false;
}

/* private */
bool 
OuterShellsExtracter::isOuter(const LinearRing& shell, std::vector<const LinearRing*>& outerShells)
{
    for (const LinearRing* outShell : outerShells) {
      if (covers(*outShell, shell)) {
        return false;
      }
    }
    return true;
}

/* private */
bool 
OuterShellsExtracter::covers(const LinearRing& shellA, const LinearRing& shellB)
{
    //-- if shellB envelope is not covered then shell is not covered
    if (! shellA.getEnvelopeInternal()->covers(shellB.getEnvelopeInternal()))
      return false;
    //-- if a shellB point lies inside shellA, shell is covered (since shells do not overlap)
    if (isPointInRing(shellB, shellA))
      return true;
    return false;
}

bool 
OuterShellsExtracter::isPointInRing(const LinearRing& shell, const LinearRing& shellRing)
{
    //TODO: optimize this with cached index
    const CoordinateXY* pt = shell.getCoordinate();
    return PointLocation::isInRing(*pt, shellRing.getCoordinatesRO());
}

void 
OuterShellsExtracter::extractShellRings(const Geometry& polygons, std::vector<const LinearRing*>& shells)
{
    shells.clear();
    for (std::size_t i = 0; i < polygons.getNumGeometries(); i++) {
        const Geometry* consGeom = polygons.getGeometryN(i);
        const Polygon* consPoly = static_cast<const Polygon*>(consGeom);
        const LinearRing* lr = consPoly->getExteriorRing();
        shells.push_back(lr);
    }
}


} // geos::algorithm::hull
} // geos::algorithm
} // geos