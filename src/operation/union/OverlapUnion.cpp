/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/union/OverlapUnion.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/util/GeometryCombiner.h>
#include <geos/util/TopologyException.h>


namespace geos {
namespace operation {
namespace geounion {

// https://github.com/locationtech/jts/blob/master/modules/core/src/main/java/org/locationtech/jts/operation/union/OverlapUnion.java

// xxx OverlapUnion::union leaks like a seive, check cascadedunion implementation for use of unique_ptr

/* public */
geom::Geometry*
OverlapUnion::union()
{
    geom::Envelope overlapEnv = overlapEnvelope(g0, g1);
    /**
     * If no overlap, can just combine the geometries
     */
    if (overlapEnv.isNull()) {
        geom::Geometry* g0Copy = g0.clone();
        geom::Geometry* g1Copy = g1.clone();
        return util::GeometryCombiner::combine(g0Copy, g1Copy);
    }

    std::vector<geom::Geometry*> disjointPolys;

    geom::Geometry* g0Overlap = extractByEnvelope(overlapEnv, g0, disjointPolys);
    geom::Geometry* g1Overlap = extractByEnvelope(overlapEnv, g1, disjointPolys);

    // std::out << "# geoms in common: " << intersectingPolys.size() << std::endl;
    geom::Geometry* unionGeom = unionFull(g0Overlap, g1Overlap);
    geom::Geometry* result = nullptr;
    isUnionSafe = isBorderSegmentsSame(unionGeom, overlapEnv);
    if (!isUnionSafe) {
      // overlap union changed border segments... need to do full union
      // std::out <<  "OverlapUnion: Falling back to full union" << std::endl;
      result = unionFull(g0, g1);
    }
    else {
      // std::out << "OverlapUnion: fast path" << std::endl;
      result = combine(unionGeom, disjointPolys);
    }
    return result;
}

/* private static */
geom::Envelope
overlapEnvelope(const geom::Geometry* g0, const geom::Geometry* g1)
{}

/* private */
geom::Geometry*
OverlapUnion::combine(const geom::Geometry* unionGeom, std::vector<geom::Geometry*> disjointPolys)
{}

/* private */
geom::Geometry*
OverlapUnion::extractByEnvelope(const geom::Envelope& env, const geom::Geometry* geom, std::vector<geom::Geometry*>& disjointGeoms)
{}

/* private */
geom::Geometry*
OverlapUnion::unionFull(const geom::Geometry* geom0, const geom::Geometry* geom1)
{}

/* private static */
geom::Geometry*
unionBuffer(const geom::Geometry* g0, const geom::Geometry* g1)
{}

/* private */
bool
OverlapUnion::isBorderSegmentsSame(const geom::Geometry* result, const geom::Envelope& env)
{}

/* private */
bool
OverlapUnion::isEqual(std::vector<geom::LineSegment*> segs0, std::vector<geom::LineSegment*> segs1)
{}

/* private */
std::vector<geom::LineSegment*>
OverlapUnion::extractBorderSegments(const geom::Geometry* geom0, const geom::Geometry* geom1, const geom::Envelope& env)
{}

/* private static */
bool
intersects(const geom::Envelope& env, const Coordinate& p0, const Coordinate& p1)
{}

/* private static */
bool
containsProperly(const geom::Envelope& env, const Coordinate& p0, const Coordinate& p1)
{}

/* private static */
void
extractBorderSegments(const geom::Geometry* geom, const geom::Envelope& env, std::vector<geom::LineSegment*> segs)
{}



}
}
}
