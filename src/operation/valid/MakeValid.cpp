/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright 2009-2010 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2019 Even Rouault <even.rouault@spatialys.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 **********************************************************************
 *
 * Ported from rtgeom_geos.c from
 *   rttopo - topology library
 *   http://git.osgeo.org/gitea/rttopo/librttopo
 * with relicensing from GPL to LGPL with Copyright holder permission.
 *
 **********************************************************************/

#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/valid/IsValidOp.h>

#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/polygonize/BuildArea.h>
#include <geos/operation/union/UnaryUnionOp.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/cluster/UnionFind.h>
#include <geos/util/Interrupt.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/util/UnsupportedOperationException.h>


// std
#include <cassert>
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

using namespace geos::geom;
using geos::operation::overlayng::OverlayNG;
using geos::operation::overlayng::OverlayNGRobust;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid


static std::unique_ptr<geom::Geometry>
makeValidSymDifference(const geom::Geometry* g0, const geom::Geometry* g1)
{
    return OverlayNGRobust::Overlay(g0, g1, OverlayNG::SYMDIFFERENCE);
}

static std::unique_ptr<geom::Geometry>
makeValidDifference(const geom::Geometry* g0, const geom::Geometry* g1)
{
    return OverlayNGRobust::Overlay(g0, g1, OverlayNG::DIFFERENCE);
}

static std::unique_ptr<geom::Geometry>
makeValidUnion(const geom::Geometry* g0, const geom::Geometry* g1)
{
    return OverlayNGRobust::Overlay(g0, g1, OverlayNG::UNION);
}

/*
 * Fully node given linework
 */
static std::unique_ptr<geom::Geometry>
nodeLineWithFirstCoordinate(const geom::Geometry* geom)
{
  /*
   * Union with first geometry point, obtaining full noding
   * and dissolving of duplicated repeated points
   *
   * TODO: substitute this with UnaryUnion?
   */

  if( geom->isEmpty() )
      return nullptr;

  const auto geomType = geom->getGeometryTypeId();
  assert( geomType == GEOS_LINESTRING || geomType == GEOS_MULTILINESTRING );

  std::unique_ptr<geom::Geometry> point;
  if( geomType == GEOS_LINESTRING ) {
      auto line = detail::down_cast<const geom::LineString*>(geom);
      point = line->getPointN(0);
  } else {
      auto mls = detail::down_cast<const geom::MultiLineString*>(geom);
      auto line = mls->getGeometryN(0);
      assert(line);
      point = line->getPointN(0);
  }

  return makeValidUnion(geom, point.get());
}


static std::unique_ptr<geom::Geometry> MakeValidLine(const geom::LineString* line)
{
    return nodeLineWithFirstCoordinate(line);
}

static std::unique_ptr<geom::Geometry> MakeValidMultiLine(const geom::MultiLineString* mls)
{
    std::vector<std::unique_ptr<geom::Geometry>> points;
    std::vector<std::unique_ptr<geom::Geometry>> lines;

    for(const auto& subgeom: *mls) {
        auto line = detail::down_cast<const geom::LineString*>(subgeom.get());
        auto validSubGeom = MakeValidLine(line);
        if( !validSubGeom || validSubGeom->isEmpty() ) {
            continue;
        }
        auto validLineType = validSubGeom->getGeometryTypeId();
        if( validLineType == GEOS_POINT ) {
            points.emplace_back(std::move(validSubGeom));
        }
        else if( validLineType == GEOS_LINESTRING ) {
            lines.emplace_back(std::move(validSubGeom));
        } else if( validLineType == GEOS_MULTILINESTRING ) {
            auto mlsValid = detail::down_cast<const geom::MultiLineString*>(validSubGeom.get());
            for(const auto& subgeomMlsValid: *mlsValid) {
                lines.emplace_back(subgeomMlsValid->clone());
            }
        } else {
            throw util::UnsupportedOperationException();
        }
    }

    std::unique_ptr<geom::Geometry> pointsRet;
    if( !points.empty() ) {
        if( points.size() > 1 ) {
            pointsRet = mls->getFactory()->createMultiPoint(std::move(points));
        } else {
            pointsRet = std::move(points[0]);
        }
    }

    std::unique_ptr<geom::Geometry> linesRet;
    if( !lines.empty() ) {
        if( lines.size() > 1 ) {
            linesRet = mls->getFactory()->createMultiLineString(std::move(lines));
        } else {
            linesRet = std::move(lines[0]);
        }
    }

    if( pointsRet && linesRet ) {
        std::vector<std::unique_ptr<Geometry>> geoms(2);
        geoms[0] = std::move(pointsRet);
        geoms[1] = std::move(linesRet);
        return mls->getFactory()->createGeometryCollection(std::move(geoms));
    } else if( pointsRet ) {
        return pointsRet;
    } else if( linesRet ) {
        return linesRet;
    }

    return nullptr;
}

static std::unique_ptr<geom::Geometry> extractUniquePoints(const geom::Geometry* geom)
{

    // Code taken from GEOSGeom_extractUniquePoints_r()

    /* 1: extract points */
    std::vector<const geom::Coordinate*> coords;
    geos::util::UniqueCoordinateArrayFilter filter(coords);
    geom->apply_ro(&filter);

    /* 2: for each point, create a geometry and put into a vector */
    std::vector<std::unique_ptr<Geometry>> points;
    points.reserve(coords.size());
    const GeometryFactory* factory = geom->getFactory();
    for(const Coordinate* c : coords) {
        points.emplace_back(factory->createPoint(*c));
    }

    /* 3: create a multipoint */
    return factory->createMultiPoint(std::move(points));
}

static std::unique_ptr<geom::Geometry> MakeValidPoly(const geom::Geometry* geom)
{
    assert( geom->getGeometryTypeId() == GEOS_POLYGON ||
            geom->getGeometryTypeId() == GEOS_MULTIPOLYGON );

    std::unique_ptr<geom::Geometry> bound(geom->getBoundary());
    if( !bound )
        return nullptr;

    /* Use noded boundaries as initial "cut" edges */
    auto cut_edges = nodeLineWithFirstCoordinate(bound.get());
    if( !cut_edges )
        return nullptr;

    /* NOTE: the noding process may drop lines collapsing to points.
    *       We want to retrieve any of those */
    auto pi = extractUniquePoints(bound.get());
    auto po = extractUniquePoints(cut_edges.get());
    std::unique_ptr<geom::Geometry> collapse_points = makeValidDifference(pi.get(), po.get());
    assert(collapse_points);
    pi.reset();
    po.reset();

    /* And use an empty geometry as initial "area" */
    const GeometryFactory* factory = geom->getFactory();
    std::unique_ptr<geom::Geometry> area(factory->createPolygon());
    assert(area);

    /*
    * See if an area can be build with the remaining edges
    * and if it can, symdifference with the original area.
    * Iterate this until no more polygons can be created
    * with left-over edges.
    */
    while( cut_edges->getNumGeometries() ) {

        GEOS_CHECK_FOR_INTERRUPTS();

        // ASSUMPTION: cut_edges should already be fully noded
        auto new_area = geos::operation::polygonize::BuildArea().build(cut_edges.get());
        assert(new_area); // never return nullptr, but exception
        if( new_area->isEmpty() ) {
            /* no more rings can be built with these edges */
            break;
        }

        // We succeeded in building a ring !
        // Save the new ring boundaries first (to compute further cut edges later)
        std::unique_ptr<geom::Geometry> new_area_bound = new_area->getBoundary();
        assert(new_area_bound);

        // Now symdif new and old area
        std::unique_ptr<geom::Geometry> symdif = makeValidSymDifference(area.get(), new_area.get());
        assert(symdif);

        GEOS_CHECK_FOR_INTERRUPTS();

        area = std::move(symdif);

        /*
        * Now let's re-set cut_edges with what's left
        * from the original boundary.
        * ASSUMPTION: only the previous cut-edges can be
        *             left, so we don't need to reconsider
        *             the whole original boundaries
        *
        * NOTE: this is an expensive operation.
        *
        */
        std::unique_ptr<geom::Geometry> new_cut_edges = makeValidDifference(cut_edges.get(), new_area_bound.get());
        assert(new_cut_edges);

        cut_edges = std::move(new_cut_edges);
    }

    std::vector<std::unique_ptr<Geometry>> vgeoms(3);
    unsigned int nvgeoms=0;

    if( !area->isEmpty() ) {
        vgeoms[nvgeoms++] = std::move(area);
    }
    if( !cut_edges->isEmpty() ) {
        vgeoms[nvgeoms++] = std::move(cut_edges);
    }
    if( !collapse_points->isEmpty() ) {
        vgeoms[nvgeoms++] = std::move(collapse_points);
    }

    if( nvgeoms == 1 ) {
        /* Return cut edges */
        return std::move(vgeoms[0]);
    }

    /* Collect areas and lines (if any line) */
    vgeoms.resize(nvgeoms);
    return factory->createGeometryCollection(std::move(vgeoms));
}

static std::unique_ptr<geom::Geometry> MakeValidCollection(const geom::GeometryCollection* coll)
{
    std::vector<std::unique_ptr<Geometry>> validGeoms;
    for(const auto& geom: *coll) {
        validGeoms.push_back(MakeValid().build(geom.get()));
    }
    return coll->getFactory()->createGeometryCollection(std::move(validGeoms));
}

namespace {

bool
isNonInteractingEnv(const geom::Envelope& env)
{
    return env.isNull() ||
           ! (std::isfinite(env.getMinX()) && std::isfinite(env.getMaxX()) &&
              std::isfinite(env.getMinY()) && std::isfinite(env.getMaxY()));
}

/*
 * Partition the polygons of a MultiPolygon into groups whose envelopes
 * are connected (envelopes intersecting, touching included).
 *
 * Polygons whose envelope is empty or not finite cannot interact with any
 * other polygon, so they are excluded from the sweep and returned as
 * single-element groups. This also keeps the sort comparator away from
 * envelopes without well-defined ordinates.
 *
 * Only polygons in different groups may be processed independently:
 * polygons sharing an envelope may still touch or overlap and must be
 * made valid together.
 */
std::vector<std::vector<std::size_t>>
envelopeConnectedComponents(const geom::MultiPolygon* mp)
{
    std::size_t n = mp->getNumGeometries();
    cluster::UnionFind sets(n);

    std::vector<const geom::Envelope*> envs(n);
    for(std::size_t i = 0; i < n; i++) {
        envs[i] = mp->getGeometryN(i)->getEnvelopeInternal();
    }

    // Sweep polygons sorted by envelope minimum X, keeping an active set of
    // envelopes that still extend to the current minimum X. Expiry is by
    // maxX, independently of position in the minX-sorted order: a long-span
    // envelope must not pin expired neighbors in the window. Only the X-live
    // set can intersect the current polygon, so the number of envelope tests
    // is proportional to true X-overlapping pairs (fast when parts are
    // spatially separated; quadratic when very many envelopes overlap).
    //
    // A spatial-index clustering such as operation::cluster::
    // EnvelopeIntersectsClusterFinder would also be correct, but for the
    // many tiny disjoint envelopes this operation is aimed at it builds
    // and queries an STRtree and materializes (and sorts) every candidate
    // hit per part, while the sweep below only ever looks at the X-live
    // set. Non-finite envelopes must be excluded either way, which is
    // done up front so the sort never sees them.
    std::vector<std::size_t> order;
    order.reserve(n);
    for(std::size_t i = 0; i < n; i++) {
        if(!isNonInteractingEnv(*envs[i])) {
            order.push_back(i);
        }
    }

    std::sort(order.begin(), order.end(),
              [&envs](std::size_t a, std::size_t b)
    {
        if(envs[a]->getMinX() != envs[b]->getMinX()) {
            return envs[a]->getMinX() < envs[b]->getMinX();
        }
        return envs[a]->getMinY() < envs[b]->getMinY();
    });

    std::vector<std::size_t> active;
    active.reserve(order.size());
    for(std::size_t hi = 0; hi < order.size(); hi++) {
        std::size_t i = order[hi];
        const double minX = envs[i]->getMinX();
        // Drop envelopes whose maxX is strictly left of the current minX.
        // Touching (maxX == minX) stays, matching Envelope::intersects.
        std::size_t w = 0;
        for(std::size_t k = 0; k < active.size(); k++) {
            if(envs[active[k]]->getMaxX() >= minX) {
                active[w++] = active[k];
            }
        }
        active.resize(w);
        for(std::size_t j : active) {
            if(envs[i]->intersects(envs[j])) {
                sets.join(i, j);
            }
        }
        active.push_back(i);
    }

    // Group indices by cluster root, preserving polygon order within groups
    // and ordering groups by first appearance.
    std::vector<std::size_t> groupId(n, n); // n = "unassigned"
    std::vector<std::vector<std::size_t>> components;
    for(std::size_t i = 0; i < n; i++) {
        std::size_t root = sets.find(i);
        if(groupId[root] == n) {
            groupId[root] = components.size();
            components.emplace_back();
        }
        components[groupId[root]].push_back(i);
    }

    return components;
}

/*
 * Move g into one of the result buckets by dimension, flattening
 * multi-geometries and collections. Ownership of g is consumed, and
 * children of multi-geometries are moved out via releaseGeometries()
 * rather than cloned. Empty elements are dropped; MakeValidPoly never
 * emits them.
 */
void
collectResult(std::unique_ptr<geom::Geometry> g,
              std::vector<std::unique_ptr<geom::Polygon>>& areas,
              std::vector<std::unique_ptr<geom::LineString>>& cutEdges,
              std::vector<std::unique_ptr<geom::Point>>& collapsePoints)
{
    if(! g || g->isEmpty()) {
        return;
    }

    switch(g->getGeometryTypeId()) {
        case GEOS_POLYGON: {
            areas.emplace_back(static_cast<geom::Polygon*>(g.release()));
            break;
        }
        case GEOS_MULTIPOLYGON:
        case GEOS_MULTILINESTRING:
        case GEOS_MULTIPOINT:
        case GEOS_GEOMETRYCOLLECTION: {
            // All collection types: move the children out and classify
            // each one. Children of MultiPolygon/MultiLineString/
            // MultiPoint are exactly Polygon/LineString/Point.
            auto coll = detail::down_cast<geom::GeometryCollection*>(g.get());
            auto elems = coll->releaseGeometries();
            for(auto& e : elems) {
                collectResult(std::move(e), areas, cutEdges, collapsePoints);
            }
            break;
        }
        case GEOS_LINESTRING: {
            cutEdges.emplace_back(static_cast<geom::LineString*>(g.release()));
            break;
        }
        case GEOS_POINT: {
            collapsePoints.emplace_back(static_cast<geom::Point*>(g.release()));
            break;
        }
        default: {
            throw util::UnsupportedOperationException();
        }
    }
}

/*
 * Make valid a MultiPolygon whose polygons fall into several
 * envelope-connected components.
 *
 * Polygons from different components have disjoint envelopes, so they
 * cannot interact: each component is made valid independently, a valid
 * component being returned as-is by the usual fast path.
 *
 * The per-component results are then merged back into the same
 * area/cut-edges/collapse-points structure MakeValidPoly would have
 * returned for the whole MultiPolygon.
 */
std::unique_ptr<geom::Geometry>
MakeValidMultiPolygonComponents(const geom::MultiPolygon* mp,
                                std::vector<std::vector<std::size_t>>& components)
{
    const GeometryFactory* factory = mp->getFactory();

    std::vector<std::unique_ptr<geom::Polygon>> areas;
    std::vector<std::unique_ptr<geom::LineString>> cutEdges;
    std::vector<std::unique_ptr<geom::Point>> collapsePoints;

    for(const auto& component : components) {
        GEOS_CHECK_FOR_INTERRUPTS();

        if(component.size() == 1) {
            // Single polygon: read it directly from the input. A valid
            // polygon still needs a clone for the result, but an invalid
            // one can be handed to MakeValidPoly through a const pointer
            // without copying it first.
            const geom::Geometry* src = mp->getGeometryN(component[0]);
            assert(src->getGeometryTypeId() == GEOS_POLYGON);

            IsValidOp ivo(src);
            if(ivo.getValidationError() == nullptr) {
                collectResult(src->clone(), areas, cutEdges, collapsePoints);
            } else {
                collectResult(MakeValidPoly(src), areas, cutEdges, collapsePoints);
            }
        } else {
            // Several interacting polygons must be made valid together;
            // they are moved into an owned MultiPolygon for that.
            std::vector<std::unique_ptr<geom::Polygon>> polys;
            polys.reserve(component.size());
            for(std::size_t idx : component) {
                polys.push_back(detail::down_cast<const geom::Polygon*>(
                    mp->getGeometryN(idx))->clone());
            }
            auto part = factory->createMultiPolygon(std::move(polys));

            // Valid components pass through unchanged, invalid ones go
            // through the polygonal MakeValid algorithm.
            IsValidOp ivo(part.get());
            if(ivo.getValidationError() == nullptr) {
                collectResult(std::move(part), areas, cutEdges, collapsePoints);
            } else {
                collectResult(MakeValidPoly(part.get()),
                              areas, cutEdges, collapsePoints);
            }
        }
    }

    std::vector<std::unique_ptr<Geometry>> vgeoms(3);
    unsigned int nvgeoms = 0;

    if(! areas.empty()) {
        if(areas.size() == 1) {
            vgeoms[nvgeoms++] = std::move(areas[0]);
        } else {
            vgeoms[nvgeoms++] = factory->createMultiPolygon(std::move(areas));
        }
    }
    if(! cutEdges.empty()) {
        if(cutEdges.size() == 1) {
            vgeoms[nvgeoms++] = std::move(cutEdges[0]);
        } else {
            vgeoms[nvgeoms++] = factory->createMultiLineString(std::move(cutEdges));
        }
    }
    if(! collapsePoints.empty()) {
        if(collapsePoints.size() == 1) {
            vgeoms[nvgeoms++] = std::move(collapsePoints[0]);
        } else {
            vgeoms[nvgeoms++] = factory->createMultiPoint(std::move(collapsePoints));
        }
    }

    if(nvgeoms == 1) {
        return std::move(vgeoms[0]);
    }

    vgeoms.resize(nvgeoms);
    return factory->createGeometryCollection(std::move(vgeoms));
}

} // namespace

/** Return a valid version of the input geometry. */
std::unique_ptr<geom::Geometry> MakeValid::build(const geom::Geometry* geom)
{

    IsValidOp ivo(geom);
    if( ivo.getValidationError() == nullptr ) {
        return std::unique_ptr<geom::Geometry>(geom->clone());
    }

    auto typeId = geom->getGeometryTypeId();
    if( typeId == GEOS_LINESTRING ) {
        auto lineString = detail::down_cast<const LineString*>(geom);
        return MakeValidLine(lineString);
    }
    if( typeId == GEOS_MULTILINESTRING ) {
        auto mls = detail::down_cast<const MultiLineString*>(geom);
        return MakeValidMultiLine(mls);
    }
    if( typeId == GEOS_POLYGON ) {
        return MakeValidPoly(geom);
    }
    if( typeId == GEOS_MULTIPOLYGON ) {
        auto mp = detail::down_cast<const MultiPolygon*>(geom);
        auto components = envelopeConnectedComponents(mp);
        if( components.size() == 1 ) {
            // All polygons potentially interact: process as a whole.
            return MakeValidPoly(geom);
        }
        return MakeValidMultiPolygonComponents(mp, components);
    }
    if( typeId == GEOS_GEOMETRYCOLLECTION ) {
        auto coll = detail::down_cast<const GeometryCollection*>(geom);
        return MakeValidCollection(coll);
    }

    throw util::UnsupportedOperationException();
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

