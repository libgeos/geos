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


#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/PolygonExtracter.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/polygon/PolygonTriangulator.h>
#include <geos/triangulate/polygon/PolygonHoleJoiner.h>
#include <geos/triangulate/polygon/PolygonEarClipper.h>



namespace geos {
namespace triangulate {
namespace polygon {


/* public static */
std::unique_ptr<Geometry>
PolygonTriangulator::triangulate(const Geometry* geom)
{
    PolygonTriangulator clipper(geom);
    return clipper.compute();
}

/* private */
std::unique_ptr<Geometry>
PolygonTriangulator::compute()
{
    // Short circuit empty case
    if(inputGeom->isEmpty()) {
        auto gf = inputGeom->getFactory();
        return gf->createGeometryCollection();
    }

    std::vector<const Polygon*> polys;
    geom::util::PolygonExtracter::getPolygons(*inputGeom, polys);

    TriList<Tri> triList;
    for (const Polygon* poly : polys) {
        // Skip empty component polygons
        if (poly->isEmpty())
            continue;
        triangulatePolygon(poly, triList);
    }
    return triList.toGeometry(geomFact);
}

/* private */
void
PolygonTriangulator::triangulatePolygon(const Polygon* poly, TriList<Tri>& triList)
{
    auto polyShell = PolygonHoleJoiner::join(poly);
    PolygonEarClipper::triangulate(*polyShell, triList);
    //Tri::validate(triList);
    return;
}


} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

