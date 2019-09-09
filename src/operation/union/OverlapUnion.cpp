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

using namespace geom;

/* public */
Geometry*
OverlapUnion::union()
{
    Envelope overlapEnv = overlapEnvelope(g0, g1);
    /**
     * If no overlap, can just combine the geometries
     */
    if (overlapEnv.isNull()) {
        Geometry* g0Copy = g0.clone();
        Geometry* g1Copy = g1.clone();
        return util::GeometryCombiner::combine(g0Copy, g1Copy);
    }

    std::vector<Geometry*> disjointPolys;

    Geometry* g0Overlap = extractByEnvelope(overlapEnv, g0, disjointPolys);
    Geometry* g1Overlap = extractByEnvelope(overlapEnv, g1, disjointPolys);

    // std::out << "# geoms in common: " << intersectingPolys.size() << std::endl;
    Geometry* unionGeom = unionFull(g0Overlap, g1Overlap);
    Geometry* result = nullptr;
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
Envelope
overlapEnvelope(const Geometry* g0, const Geometry* g1)
{
    const Envelope* g0Env = g0->getEnvelopeInternal();
    const Envelope* g1Env = g1->getEnvelopeInternal();
    Envelope overlapEnv;
    g0Env->intersection(g1Env, overlapEnv);
    return overlapEnv;
}

/* private */
Geometry*
OverlapUnion::combine(Geometry* unionGeom, std::vector<Geometry*>& disjointPolys)
{
    if (disjointPolys.size() <= 0)
        return unionGeom;

    disjointPolys.push_back(unionGeom);
    return GeometryCombiner::combine(disjointPolys).get();
}

/* private */
Geometry*
OverlapUnion::extractByEnvelope(const Envelope& env, const Geometry* geom, std::vector<Geometry*>& disjointGeoms)
{
    std::vector<Geometry*> intersectingGeoms;
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        Geometry* elem = geom->getGeometryN(i);
        if (elem->getEnvelopeInternal()->intersects(env)) {
            intersectingGeoms.push_back(elem);
        }
        else {
            Geometry* copy = elem->copy();
            disjointGeoms.push_back(copy);
        }
    }
    return geomFactory->buildGeometry(intersectingGeoms);
}

/* private */
Geometry*
OverlapUnion::unionFull(const Geometry* geom0, const Geometry* geom1)
{
    try {
        return geom0->union(geom1);
    }
    catch (util::TopologyException ex) {
        /**
         * If the overlay union fails,
         * try a buffer union, which often succeeds
         */
        return unionBuffer(geom0, geom1);
    }
}

/* private static */
Geometry*
unionBuffer(const Geometry* g0, const Geometry* g1)
{
    std::vector<Geometry*> geoms = {g0 g1};
    GeometryFactory* factory = g0->getFactory();
    Geometry* gColl = factory->createGeometryCollection(geoms);
    Geometry* unionGeom = gColl->buffer(0.0);
    return unionGeom;
}

/* private */
bool
OverlapUnion::isBorderSegmentsSame(const Geometry* result, const Envelope& env)
{
    std::vector<std::unique_ptr<LineSegment>> segsBefore = extractBorderSegments(g0, g1, env);
    std::vector<std::unique_ptr<LineSegment>> segsAfter;
    extractBorderSegments(result, env, segsAfter);

    //std::cout << ("# seg before: " << segsBefore.size() << " - # seg after: " << segsAfter.size() << std::endl;
    return isEqual(segsBefore, segsAfter);
}

/* private */
bool
OverlapUnion::isEqual(std::vector<std::unique_ptr<LineSegment>>& segs0, std::vector<std::unique_ptr<LineSegment>>& segs1)
{
    if (segs0.size() != segs1.size())
        return false;

    std::unordered_set<std::unique_ptr<LineSegment>> segIndex(segs0);

    for (auto seg : segs1) {
        if (segIndex.count(seg) == 0) {
            //std::cout <<  "Found changed border seg: " << seg << std::endl;
            return false;
        }
    }
    return true;
}

/* private */
std::vector<std::unique_ptr<LineSegment>>
OverlapUnion::extractBorderSegments(const Geometry* geom0, const Geometry* geom1, const Envelope& env)
{
    std::vector<std::unique_ptr<LineSegment>> segs;
    extractBorderSegments(geom0, env, segs);
    if (geom1 != nullptr)
        extractBorderSegments(geom1, env, segs);
    return segs;
}

/* private static */
bool
intersects(const Envelope& env, const Coordinate& p0, const Coordinate& p1)
{
    return env.intersects(p0) || env.intersects(p1);
}

/* private static */
bool
containsProperly(const Envelope& env, const Coordinate& p0, const Coordinate& p1)
{
    return containsProperly(env, p0) && containsProperly(env, p1);
}

/* private static */
void
extractBorderSegments(const Geometry* geom, const Envelope& penv, std::vector<std::unique_ptr<LineSegment>>& psegs)
{
    class BorderSegmentFilter : public CoordinateSequenceFilter {

    private:
        std::vector<std::unique_ptr<LineSegment>> segs;
        const Envelope env;

    public:

        BorderSegmentFilter(const Envelope& penv, std::vector<unique_ptr<LineSegment>> psegs)
            : env(penv),
              segs(psegs) {};

        bool
        isDone() const override { return false; }

        bool
        isGeometryChanged() const override  { return false; }

        void
        filter_rw(CoordinateSequence& seq, std::size_t i)
        {
            if (i <= 0) return;

            // extract LineSegment
            Coordinate p0, p1;
            seq.getAt(i-1, p0);
            seq.getAt(  i, p1);
            bool isBorder = intersects(env, p0, p1) && ! containsProperly(env, p0, p1);
            if (isBorder) {
                std::unique_ptr<LineSegment> seg(new LineSegment(p0, p1));
                segs.push_back(seg);
            }
        }

        void
        filter_ro(CoordinateSequence& seq, std::size_t i)
        {
            ::geos::ignore_unused_variable_warning(seq);
            ::geos::ignore_unused_variable_warning(i);
        }
    };

    BorderSegmentFilter bsf(penv, psegs);
    geom->apply_rw(bsf);

}



}
}
}
