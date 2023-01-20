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


#include <geos/geom/util/PolygonExtracter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/tri/TriangulationBuilder.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>
#include <geos/triangulate/polygon/PolygonHoleJoiner.h>
#include <geos/triangulate/polygon/PolygonEarClipper.h>
#include <geos/triangulate/polygon/TriDelaunayImprover.h>


namespace geos {
namespace triangulate {
namespace polygon {


/* public static */
std::unique_ptr<Geometry>
ConstrainedDelaunayTriangulator::triangulate(const Geometry* geom)
{
    ConstrainedDelaunayTriangulator cdt(geom);
    return cdt.compute();
}


/* private */
std::unique_ptr<Geometry>
ConstrainedDelaunayTriangulator::compute()
{
    // short circuit empty input case
    if(inputGeom->isEmpty()) {
        auto gf = inputGeom->getFactory();
        return gf->createGeometryCollection();
    }

    std::vector<const Polygon*> polys;
    geom::util::PolygonExtracter::getPolygons(*inputGeom, polys);

    std::vector<std::unique_ptr<TriList<Tri>>> allTriLists;
    for (const Polygon* poly : polys) {
        std::unique_ptr<TriList<Tri>> triList(new TriList<Tri>());
        // Skip empty component polygons
        if (poly->isEmpty())
            continue;
        triangulatePolygon(poly, *triList);
        allTriLists.emplace_back(triList.release());
    }
    return toGeometry(geomFact, allTriLists);
}

/* private static */
std::unique_ptr<Geometry>
ConstrainedDelaunayTriangulator::toGeometry(
    const geom::GeometryFactory* geomFact,
    const std::vector<std::unique_ptr<TriList<Tri>>>& allTriLists)
{
    std::vector<std::unique_ptr<Geometry>> geoms;
    for (const std::unique_ptr<TriList<Tri>>& triList: allTriLists) {
        for (const auto* tri: *triList) {
            std::unique_ptr<Geometry> geom = tri->toPolygon(geomFact);
            geoms.emplace_back(geom.release());
        }
    }
    return geomFact->createGeometryCollection(std::move(geoms));
}

/**
* Computes the triangulation of a single polygon
* and returns it as a list of {@link Tri}s.
*
* @param poly the input polygon
* @return list of Tris forming the triangulation
*/
void
ConstrainedDelaunayTriangulator::triangulatePolygon(const Polygon* poly, TriList<Tri>& triList)
{
    auto polyShell = PolygonHoleJoiner::join(poly);
    PolygonEarClipper::triangulate(*polyShell, triList);
    tri::TriangulationBuilder::build(triList);
    TriDelaunayImprover::improve(triList);
    return;
}




} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

