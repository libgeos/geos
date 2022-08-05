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

#include <geos/coverage/CoverageGapFinder.h>
#include <geos/coverage/CoverageUnion.h>


#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/PolygonExtracter.h>


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::geom::util::PolygonExtracter;
using geos::algorithm::construct::MaximumInscribedCircle;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public static */
std::unique_ptr<Geometry>
CoverageGapFinder::findGaps(std::vector<const Geometry*>& coverage, double gapWidth)
{
    CoverageGapFinder finder(coverage);
    return finder.findGaps(gapWidth);
}


/* public */
std::unique_ptr<Geometry>
CoverageGapFinder::findGaps(double gapWidth)
{
    std::unique_ptr<Geometry> geomUnion = CoverageUnion::Union(m_coverage);
    std::vector<const Polygon*> polygons;
    PolygonExtracter::getPolygons(*geomUnion, polygons);

    std::vector<std::unique_ptr<LineString>> gapLines;
    for (const Polygon* poly : polygons) {
        for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
            const LinearRing* hole = poly->getInteriorRingN(i);
            if (isGap(hole, gapWidth)) {
                auto pts = hole->getCoordinatesRO()->clone();
                auto ls = geomUnion->getFactory()->createLineString(std::move(pts));
                gapLines.emplace_back(ls.release());
            }
        }
    }
    return geomUnion->getFactory()->buildGeometry(std::move(gapLines));
}


/* private */
bool
CoverageGapFinder::isGap(const LinearRing* hole, double gapWidth)
{
    std::vector<LinearRing*> noOtherHoles;

    //-- guard against bad input
    if (gapWidth <= 0.0)
        return false;

    std::unique_ptr<Geometry> holePoly(hole->getFactory()->createPolygon(*hole, noOtherHoles));

    double tolerance = gapWidth / 100;
    //TODO: improve MIC class to allow short-circuiting when radius is larger than a value
    std::unique_ptr<geom::LineString> line = MaximumInscribedCircle::getRadiusLine(holePoly.get(), tolerance);
    double width = line->getLength() * 2;
    return width <= gapWidth;
}



} // namespace geos.coverage
} // namespace geos


