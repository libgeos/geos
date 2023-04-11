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

#include <geos/algorithm/Angle.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Triangle.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/polygon/PolygonEarClipper.h>
#include <geos/util/IllegalStateException.h>

using geos::algorithm::Orientation;
using geos::algorithm::Angle;
using geos::triangulate::tri::TriList;

namespace geos {
namespace triangulate {
namespace polygon {


/* public */
PolygonEarClipper::PolygonEarClipper(const geom::CoordinateSequence& polyShell)
    : vertex(polyShell)
    , vertexSize(polyShell.size()-1)
    , vertexFirst(0)
    , vertexCoordIndex(polyShell)
{
    vertexNext = createNextLinks(vertexSize);
    initCornerIndex();
}


/* private */
std::vector<std::size_t>
PolygonEarClipper::createNextLinks(std::size_t size) const
{
    std::vector<std::size_t> next(size);
    for (std::size_t i = 0; i < size; i++) {
        next[i] = i + 1;
    }
    next[size - 1] = 0;
    return next;
}


/* public static */
void
PolygonEarClipper::triangulate(const geom::CoordinateSequence& polyShell, TriList<Tri>& triListResult)
{
    PolygonEarClipper clipper(polyShell);
    clipper.compute(triListResult);
}


/* public */
void
PolygonEarClipper::setSkipFlatCorners(bool p_isFlatCornersSkipped)
{
    isFlatCornersSkipped  = p_isFlatCornersSkipped;
}


/* public */
void
PolygonEarClipper::compute(TriList<Tri>& triList)
{
    /**
     * Count scanned corners, to catch infinite loops
     * (which indicate an algorithm bug)
     */
    std::size_t cornerScanCount = 0;

    std::array<Coordinate, 3> corner;
    fetchCorner(corner);

    /**
     * Scan continuously around vertex ring,
     * until all ears have been found.
     */
    while (true) {
        /**
        * Non-convex corner- remove if flat, or skip
        * (a concave corner will turn into a convex corner
        * after enough ears are removed)
        */
        if (! isConvex(corner)) {
            // remove the corner if it is invalid or flat (if required)
            bool isCornerRemoved = isCornerInvalid(corner)
                || (isFlatCornersSkipped && isFlat(corner));
            if (isCornerRemoved) {
                removeCorner();
            }
            cornerScanCount++;
            if (cornerScanCount > 2 * vertexSize) {
                throw util::IllegalStateException("Unable to find a convex corner");
            }
        }
        /**
        * Convex corner - check if it is a valid ear
        */
        else if (isValidEar(cornerIndex[1], corner)) {
            triList.add(corner[0], corner[1], corner[2]);
            removeCorner();
            cornerScanCount = 0;
        }
        if (cornerScanCount > 2 * vertexSize) {
            throw util::IllegalStateException("Unable to find a valid ear");
        }

        //--- done when all corners are processed and removed
        if (vertexSize < 3) {
            return;
        }

        /**
        * Skip to next corner.
        * This is done even after an ear is removed,
        * since that creates fewer skinny triangles.
        */
        nextCorner(corner);
    }
}

/* private */
bool
PolygonEarClipper::isValidEar(std::size_t cornerIdx, const std::array<Coordinate, 3>& corner)
{
    std::size_t intApexIndex = findIntersectingVertex(cornerIdx, corner);
    //--- no intersections found
    if (intApexIndex == NO_COORD_INDEX)
        return true;
    //--- check for duplicate corner apex vertex
    if ( vertex[intApexIndex].equals2D(corner[1]) ) {
        //--- a duplicate corner vertex requires a full scan
        return isValidEarScan(cornerIdx, corner);
    }
    return false;
}


/* private */
std::size_t
PolygonEarClipper::findIntersectingVertex(std::size_t cornerIdx, const std::array<Coordinate, 3>& corner) const
{
    Envelope cornerEnv = envelope(corner);
    std::vector<std::size_t> result;
    vertexCoordIndex.query(cornerEnv, result);

    std::size_t dupApexIndex = NO_COORD_INDEX;
    //--- check for duplicate vertices
    for (std::size_t i = 0; i < result.size(); i++) {
        std::size_t vertIndex = result[i];

        if (vertIndex == cornerIdx ||
            vertIndex == vertex.size() - 1 ||
            isRemoved(vertIndex))
            continue;

        const Coordinate& v = vertex[vertIndex];
        /**
        * If another vertex at the corner is found,
        * need to do a full scan to check the incident segments.
        * This happens when the polygon ring self-intersects,
        * usually due to hold joining.
        * But only report this if no properly intersecting vertex is found,
        * for efficiency.
        */
        if (v.equals2D(corner[1])) {
            dupApexIndex = vertIndex;
        }
        //--- don't need to check other corner vertices
        else if (v.equals2D(corner[0]) || v.equals2D(corner[2])) {
            continue;
        }
        //--- this is a properly intersecting vertex
        else if (geom::Triangle::intersects(corner[0], corner[1], corner[2], v))
            return vertIndex;
    }
    if (dupApexIndex != NO_COORD_INDEX) {
        return dupApexIndex;
    }
    return NO_COORD_INDEX;
}


/* private */
bool
PolygonEarClipper::isValidEarScan(std::size_t cornerIdx, const std::array<Coordinate, 3>& corner) const
{
    double cornerAngle = Angle::angleBetweenOriented(corner[0], corner[1], corner[2]);

    std::size_t currIndex = nextIndex(vertexFirst);
    std::size_t prevIndex = vertexFirst;
    for (std::size_t i = 0; i < vertexSize; i++) {
        const Coordinate& vPrev = vertex[prevIndex];
        const Coordinate& v = vertex[currIndex];
        /**
        * Because of hole-joining vertices can occur more than once.
        * If vertex is same as corner[1],
        * check whether either adjacent edge lies inside the ear corner.
        * If so the ear is invalid.
        */
        if (currIndex != cornerIdx && v.equals2D(corner[1])) {
            const Coordinate& vNext = vertex[nextIndex(currIndex)];

            //TODO: for robustness use segment orientation instead
            double aOut = Angle::angleBetweenOriented(corner[0], corner[1], vNext);
            double aIn = Angle::angleBetweenOriented(corner[0], corner[1], vPrev);
            if (aOut > 0 && aOut < cornerAngle ) {
                return false;
            }
            if (aIn > 0 && aIn < cornerAngle) {
                return false;
            }
            if (aOut == 0 && aIn == cornerAngle) {
                return false;
            }
        }

        //--- move to next vertex
        prevIndex = currIndex;
        currIndex = nextIndex(currIndex);
    }
    return true;
}


/* private static */
Envelope
PolygonEarClipper::envelope(const std::array<Coordinate, 3>& corner)
{
    Envelope cornerEnv(corner[0], corner[1]);
    cornerEnv.expandToInclude(corner[2]);
    return cornerEnv;
}


/* private */
void
PolygonEarClipper::removeCorner()
{
    std::size_t cornerApexIndex = cornerIndex[1];
    if (vertexFirst == cornerApexIndex) {
        vertexFirst = vertexNext[cornerApexIndex];
    }
    vertexNext[cornerIndex[0]] = vertexNext[cornerApexIndex];
    vertexCoordIndex.remove(cornerApexIndex);
    vertexNext[cornerApexIndex] = NO_COORD_INDEX;
    vertexSize--;
    //-- adjust following corner indexes
    cornerIndex[1] = nextIndex(cornerIndex[0]);
    cornerIndex[2] = nextIndex(cornerIndex[1]);
}


/* private */
bool
PolygonEarClipper::isRemoved(std::size_t vertexIndex) const
{
    return NO_COORD_INDEX == vertexNext[vertexIndex];
}


/* private */
void
PolygonEarClipper::initCornerIndex()
{
    cornerIndex[0] = 0;
    cornerIndex[1] = 1;
    cornerIndex[2] = 2;
}


/* private */
void
PolygonEarClipper::fetchCorner(std::array<Coordinate, 3>& cornerVertex) const
{
    cornerVertex[0] = vertex[cornerIndex[0]];
    cornerVertex[1] = vertex[cornerIndex[1]];
    cornerVertex[2] = vertex[cornerIndex[2]];
}


/* private */
void
PolygonEarClipper::nextCorner(std::array<Coordinate, 3>& cornerVertex)
{
    if ( vertexSize < 3 ) {
        return;
    }
    cornerIndex[0] = nextIndex(cornerIndex[0]);
    cornerIndex[1] = nextIndex(cornerIndex[0]);
    cornerIndex[2] = nextIndex(cornerIndex[1]);
    fetchCorner(cornerVertex);
}


/* private */
std::size_t
PolygonEarClipper::nextIndex(std::size_t index) const
{
    return vertexNext[index];
}


/* private static */
bool
PolygonEarClipper::isConvex(const std::array<Coordinate, 3>& pts) const
{
    return Orientation::CLOCKWISE == Orientation::index(pts[0], pts[1], pts[2]);
}


/* private static */
bool
PolygonEarClipper::isFlat(const std::array<Coordinate, 3>& pts) const
{
    return Orientation::COLLINEAR == Orientation::index(pts[0], pts[1], pts[2]);
}


/* private static */
bool
PolygonEarClipper::isCornerInvalid(const std::array<Coordinate, 3>& pts) const
{
    return pts[1].equals2D(pts[0]) || pts[1].equals2D(pts[2]) || pts[0].equals2D(pts[2]);
}


/* public */
std::unique_ptr<Polygon>
PolygonEarClipper::toGeometry() const
{
    auto gf = geom::GeometryFactory::create();
    std::unique_ptr<geom::CoordinateSequence> cs(new geom::CoordinateSequence());
    std::size_t index = vertexFirst;
    for (std::size_t i = 0; i < vertexSize; i++) {
        const Coordinate& v = vertex[index];
        index = nextIndex(index);
        cs->add(v, true);
    }
    cs->closeRing();
    auto lr = gf->createLinearRing(std::move(cs));
    return gf->createPolygon(std::move(lr));
}


} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos


