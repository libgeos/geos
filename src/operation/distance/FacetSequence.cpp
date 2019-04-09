/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2016 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/distance/FacetSequence.java (f6187ee2 JTS-1.14)
 *
 **********************************************************************/

#include <geos/algorithm/Distance.h>
#include <geos/operation/distance/FacetSequence.h>

using namespace geos::geom;
using namespace geos::operation::distance;
using namespace geos::algorithm;

FacetSequence::FacetSequence(const Geometry *p_geom, const CoordinateSequence* p_pts, size_t p_start, size_t p_end) :
    geom(p_geom),
    pts(p_pts),
    start(p_start),
    end(p_end)
{
    computeEnvelope();
}

FacetSequence::FacetSequence(const CoordinateSequence* p_pts, size_t p_start, size_t p_end) :
    geom(nullptr),
    pts(p_pts),
    start(p_start),
    end(p_end)
{
    computeEnvelope();
}

size_t
FacetSequence::size() const
{
    return end - start;
}

bool
FacetSequence::isPoint() const
{
    return end - start == 1;
}

double
FacetSequence::distance(const FacetSequence& facetSeq) const
{
    bool isPointThis = isPoint();
    bool isPointOther = facetSeq.isPoint();
    double distance;

    if(isPointThis && isPointOther) {
        Coordinate pt = pts->getAt(start);
        Coordinate seqPt = facetSeq.pts->getAt(facetSeq.start);
        distance = pt.distance(seqPt);
    }
    else if(isPointThis) {
        Coordinate pt = pts->getAt(start);
        distance = computeDistancePointLine(pt, facetSeq, nullptr);
    }
    else if(isPointOther) {
        Coordinate seqPt = facetSeq.pts->getAt(facetSeq.start);
        distance = computeDistancePointLine(seqPt, *this, nullptr);
    }
    else {
        distance = computeDistanceLineLine(facetSeq, nullptr);
    }

    return distance;
}

/*
* Rather than get bent out of shape about returning a pointer
* just return the whole mess, since it only ends up holding two
* locations.
*/
std::vector<GeometryLocation>
FacetSequence::nearestLocations(const FacetSequence& facetSeq)  const
{
    Coordinate pt, seqPt;
    bool isPointThis = isPoint();
    bool isPointOther = facetSeq.isPoint();
    std::vector<GeometryLocation> locs;
    if (isPointThis && isPointOther) {
        pts->getAt(start, pt);
        facetSeq.pts->getAt(facetSeq.start, seqPt);
        GeometryLocation gl1(geom, start, pt);
        GeometryLocation gl2(facetSeq.geom, facetSeq.start, seqPt);
    }
    else if (isPointThis) {
        pts->getAt(start, pt);
        computeDistancePointLine(pt, facetSeq, &locs);
    }
    else if (isPointOther) {
        facetSeq.pts->getAt(facetSeq.start, seqPt);
        computeDistancePointLine(seqPt, *this, &locs);
        // unflip the locations
        GeometryLocation tmp = locs[0];
        locs[0] = locs[1];
        locs[1] = tmp;
    }
    else {
        computeDistanceLineLine(facetSeq, &locs);
    }
    return locs;
}

double
FacetSequence::computeDistancePointLine(const Coordinate& pt,
                                        const FacetSequence& facetSeq,
                                        std::vector<GeometryLocation> *locs) const
{
    double minDistance = std::numeric_limits<double>::infinity();
    double dist;
    Coordinate q0;
    Coordinate q1;

    for(size_t i = facetSeq.start; i < facetSeq.end - 1; i++) {
        facetSeq.pts->getAt(i, q0);
        facetSeq.pts->getAt(i + 1, q1);
        dist = Distance::pointToSegment(pt, q0, q1);
        if(dist == 0.0) {
            return dist;
        }
        if(dist < minDistance) {
            minDistance = dist;
            if (locs != nullptr) {
                updateNearestLocationsPointLine(pt, facetSeq, i, q0, q1, locs);
            }
            if(minDistance <= 0.0) {
                return minDistance;
            }
        }
    }

    return minDistance;
}

void
FacetSequence::updateNearestLocationsPointLine(const Coordinate& pt,
        const FacetSequence& facetSeq, int i,
        const Coordinate& q0, const Coordinate &q1,
        std::vector<GeometryLocation> *locs) const
{
    geom::LineSegment seg(q0, q1);
    Coordinate segClosestPoint;
    seg.closestPoint(pt, segClosestPoint);
    GeometryLocation gl0(geom, start, pt);
    GeometryLocation gl1(facetSeq.geom, i, segClosestPoint);
    locs->clear();
    locs->push_back(gl0);
    locs->push_back(gl1);
    return;
}

double
FacetSequence::computeDistanceLineLine(const FacetSequence& facetSeq, std::vector<GeometryLocation> *locs) const
{
    double minDistance = std::numeric_limits<double>::infinity();
    double dist;
    Coordinate p0, p1, q0, q1;

    for(size_t i = start; i < end - 1; i++) {
        pts->getAt(i, p0);
        pts->getAt(i + 1, p1);

        for(size_t j = facetSeq.start; j < facetSeq.end - 1; j++) {
            facetSeq.pts->getAt(j, q0);
            facetSeq.pts->getAt(j + 1, q1);

            dist = Distance::segmentToSegment(p0, p1, q0, q1);
            if(dist <= minDistance) {
                minDistance = dist;
                if(locs != nullptr) {
                    updateNearestLocationsLineLine(i, p0, p1, facetSeq, j, q0, q1, locs);
                }
                if(minDistance <= 0.0) return minDistance;
            }
        }
    }

    return minDistance;
}

void
FacetSequence::updateNearestLocationsLineLine(int i, const Coordinate& p0, const Coordinate& p1,
        const FacetSequence& facetSeq,
        int j, const Coordinate& q0, const Coordinate &q1,
        std::vector<GeometryLocation> *locs) const
{
    LineSegment seg0(p0, p1);
    LineSegment seg1(q0, q1);
    CoordinateSequence* closestPts = seg0.closestPoints(seg1);
    Coordinate c0, c1;
    closestPts->getAt(0, c0);
    closestPts->getAt(1, c1);
    GeometryLocation gl0(geom, i, c0);
    GeometryLocation gl1(facetSeq.geom, j, c1);
    locs->clear();
    locs->push_back(gl0);
    locs->push_back(gl1);
    return;
}

void
FacetSequence::computeEnvelope()
{
    env = Envelope();
    for(size_t i = start; i < end; i++) {
        env.expandToInclude(pts->getX(i), pts->getY(i));
    }
}

const Envelope*
FacetSequence::getEnvelope() const
{
    return &env;
}

const Coordinate*
FacetSequence::getCoordinate(size_t index) const
{
    return &(pts->getAt(start + index));
}

