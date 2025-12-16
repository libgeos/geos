/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <sstream>


using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::Dimension;
using geos::geom::Geometry;
using geos::algorithm::Orientation;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
const RelateSegmentString*
RelateSegmentString::createLine(
    const std::shared_ptr<const CoordinateSequence>& pts,
    bool isA, int elementId,
    const RelateGeometry* parent)
{
    return createSegmentString(pts, isA, Dimension::L, elementId, -1, nullptr, parent);
}


/* public static */
const RelateSegmentString*
RelateSegmentString::createRing(
    const std::shared_ptr<const CoordinateSequence>& pts,
    bool isA, int elementId, int ringId,
    const Geometry* poly, const RelateGeometry* parent)
{
    return createSegmentString(pts, isA, Dimension::A, elementId, ringId, poly, parent);
}


/* private static */
const RelateSegmentString*
RelateSegmentString::createSegmentString(
    const std::shared_ptr<const CoordinateSequence>& pts,
    bool isA, int dim, int elementId, int ringId,
    const Geometry* poly, const RelateGeometry* parent)
{
    return new RelateSegmentString(pts, isA, dim, elementId, ringId, poly, parent);
}


/* public */
NodeSection*
RelateSegmentString::createNodeSection(std::size_t segIndex, const CoordinateXY& intPt) const
{
    const CoordinateXY& c0 = getCoordinate(segIndex);
    const CoordinateXY& c1 = getCoordinate(segIndex + 1);
    bool isNodeAtVertex = intPt.equals2D(c0) || intPt.equals2D(c1);
    const CoordinateXY* prev = prevVertex(segIndex, &intPt);
    const CoordinateXY* next = nextVertex(segIndex, &intPt);
    NodeSection* a = new NodeSection(m_isA, m_dimension, m_id, m_ringId, m_parentPolygonal, isNodeAtVertex, prev, intPt, next);
    return a;
}


/* private */
const CoordinateXY*
RelateSegmentString::prevVertex(std::size_t segIndex, const CoordinateXY* pt) const
{
    const CoordinateXY& segStart = getCoordinate(segIndex);
    if (! segStart.equals2D(*pt))
        return &segStart;

    //-- pt is at segment start, so get previous vertex
    if (segIndex > 0) {
        const CoordinateXY& seg = getCoordinate(segIndex - 1);
        return &seg;
    }

    if (isClosed())
        return &(prevInRing(segIndex));

    return nullptr;
}


/* private */
const CoordinateXY*
RelateSegmentString::nextVertex(std::size_t segIndex, const CoordinateXY* pt) const
{
    const CoordinateXY& segEnd = getCoordinate(segIndex + 1);
    if (! segEnd.equals2D(*pt))
        return &segEnd;

    //-- pt is at seg end, so get next vertex
    if (size() == 2 && segIndex == INDEX_UNKNOWN) {
        const CoordinateXY& seg = getCoordinate(0);
        return &seg;
    }

    if (segIndex < size() - 2) {
        const CoordinateXY& seg = getCoordinate(segIndex + 2);
        return &seg;
    }

    if (isClosed())
        return &(SegmentString::nextInRing(segIndex + 1));

    //-- segstring is not closed, so there is no next segment
    return nullptr;
}


/* public */
bool
RelateSegmentString::isContainingSegment(std::size_t segIndex, const CoordinateXY* pt) const
{
    //-- intersection is at segment start vertex - process it
    const CoordinateXY& c0 = getCoordinate(segIndex);
    if (pt->equals2D(c0))
        return true;
    const CoordinateXY& c1 = getCoordinate(segIndex+1);
    if (pt->equals2D(c1)) {
        bool isFinalSegment = segIndex == size() - 2;
        if (isClosed() || ! isFinalSegment)
            return false;
        //-- for final segment, process intersections with final endpoint
        return true;
    }
    //-- intersection is interior - process it
    return true;
}




} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


