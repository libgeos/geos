/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2022 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageEdge.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineSegment.h>
#include <geos/util/IllegalStateException.h>


using geos::geom::Coordinate;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::LineSegment;


namespace geos {     // geos
namespace coverage { // geos.coverage

/* public static */
std::unique_ptr<CoverageEdge>
CoverageEdge::createEdge(const LinearRing* ring)
{
    auto pts = extractEdgePoints(ring, 0, ring->getNumPoints() - 1);
    return detail::make_unique<CoverageEdge>(std::move(pts), true);
}

/* public static */
std::unique_ptr<CoverageEdge>
CoverageEdge::createEdge(const LinearRing* ring,
    std::size_t start, std::size_t end)
{
    auto pts = extractEdgePoints(ring, start, end);
    return detail::make_unique<CoverageEdge>(std::move(pts), false);
}

/* public static */
std::unique_ptr<MultiLineString>
CoverageEdge::createLines(
    const std::vector<CoverageEdge*>& edges,
    const GeometryFactory* geomFactory)
{
    std::vector<std::unique_ptr<LineString>> lines;
    for (const CoverageEdge* edge : edges) {
        auto cs = edge->getCoordinates()->clone();
        auto ls = geomFactory->createLineString(std::move(cs));
        lines.push_back(std::move(ls));
    }
    return geomFactory->createMultiLineString(std::move(lines));
}

/* public */
std::unique_ptr<LineString>
CoverageEdge::toLineString(const GeometryFactory* geomFactory)
{
    const CoordinateSequence* cs = getCoordinates();
    return geomFactory->createLineString(cs->clone());
}

/* private static */
std::unique_ptr<CoordinateSequence>
CoverageEdge::extractEdgePoints(const LinearRing* ring,
    std::size_t start, std::size_t end)
{
    auto pts = detail::make_unique<CoordinateSequence>();
    std::size_t size = start < end
                  ? end - start + 1
                  : ring->getNumPoints() - start + end;
    std::size_t iring = start;
    auto cs = ring->getCoordinatesRO();
    for (std::size_t i = 0; i < size; i++) {
        pts->add(cs->getAt(iring));
        iring += 1;
        if (iring >= ring->getNumPoints())
            iring = 1;
    }
    return pts;
}


/* public static */
LineSegment
CoverageEdge::key(const LinearRing* ring)
{
    const CoordinateSequence* pts = ring->getCoordinatesRO();
    // find lowest vertex index
    std::size_t indexLow = 0;
    for (std::size_t i = 1; i < pts->size() - 1; i++) {
        if (pts->getAt(indexLow).compareTo(pts->getAt(i)) < 0)
            indexLow = i;
    }
    const Coordinate& key0 = pts->getAt(indexLow);
    // find distinct adjacent vertices
    const Coordinate& adj0 = findDistinctPoint(pts, indexLow, true, key0);
    const Coordinate& adj1 = findDistinctPoint(pts, indexLow, false, key0);
    const Coordinate& key1 = adj0.compareTo(adj1) < 0 ? adj0 : adj1;
    return LineSegment(key0, key1);
}


/* public static */
LineSegment
CoverageEdge::key(const LinearRing* ring,
    std::size_t start, std::size_t end)
{
    const CoordinateSequence* pts = ring->getCoordinatesRO();
    //-- endpoints are distinct in a line edge
    const Coordinate& end0 = pts->getAt(start);
    const Coordinate& end1 = pts->getAt(end);
    bool isForward = 0 > end0.compareTo(end1);
    const Coordinate* key0;
    const Coordinate* key1;
    if (isForward) {
        key0 = &end0;
        key1 = &findDistinctPoint(pts, start, true, *key0);
    }
    else {
        key0 = &end1;
        key1 = &findDistinctPoint(pts, end, false, *key0);
    }
    return LineSegment(*key0, *key1);
}

/* private static */
const Coordinate&
CoverageEdge::findDistinctPoint(
    const CoordinateSequence* pts,
    std::size_t index,
    bool isForward,
    const Coordinate& pt)
{
    std::size_t i = index;
    std::size_t endIndex = pts->size()-1;
    do {
        if (! pts->getAt(i).equals2D(pt)) {
            return pts->getAt(i);
        }
        // increment index with wrapping
        if (isForward) {
            i = (i == endIndex) ? 0 : (i+1);
        }
        else {
            i = (i == 0) ? endIndex : (i-1);
        }

    } while (i != index);
    throw util::IllegalStateException("Edge does not contain distinct points");
}



// /* public */
// std::string toString()
// {
//     return WKTWriter::toLineString(pts);
// }



} // namespace geos.coverage
} // namespace geos


