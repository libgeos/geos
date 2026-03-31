/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageEdges.h>
#include <geos/coverage/CoverageRingEdges.h>
#include <geos/coverage/CoverageEdge.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/util/PolygonExtracter.h>

namespace geos {      // geos
namespace coverage { // geos::coverage

/* public static */
std::unique_ptr<geom::Geometry>
CoverageEdges::GetEdges(const std::vector<const geom::Geometry*>& coverage, int type)
{
    if (coverage.empty()) {
        return nullptr;
    }

    const geom::GeometryFactory* factory = coverage[0]->getFactory();
    CoverageRingEdges cre(coverage);
    std::vector<CoverageEdge*> selectedEdges;

    for (auto* edge : cre.getEdges()) {
        if (type == 2) { // INTERIOR
            if (edge->getRingCount() > 1)
                selectedEdges.push_back(edge);
        }
        else if (type == 1) { // EXTERIOR
            if (edge->getRingCount() == 1)
                selectedEdges.push_back(edge);
        }
        else { // ALL (0 and default)
            selectedEdges.push_back(edge);
        }
    }

    return CoverageEdge::createLines(selectedEdges, factory);
}

/* public static */
std::unique_ptr<geom::Geometry>
CoverageEdges::GetEdges(const geom::Geometry* input, int type)
{
    std::vector<const geom::Polygon*> polygons;
    geom::util::PolygonExtracter::getPolygons(*input, polygons);

    if (polygons.empty()) {
        return input->getFactory()->createMultiLineString();
    }

    std::vector<const geom::Geometry*> coverage;
    for (const auto* poly : polygons) {
        coverage.push_back(poly);
    }

    return GetEdges(coverage, type);
}


} // namespace geos::coverage
} // namespace geos
