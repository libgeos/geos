/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/union/CascadedPolygonUnion.java r487 (JTS-1.12+)
 * Includes custom code to deal with https://trac.osgeo.org/geos/ticket/837
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/HeuristicOverlay.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Polygon.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/union/CascadedPolygonUnion.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/valid/IsSimpleOp.h>
#include <geos/util/TopologyException.h>

// std
#include <cassert>
#include <cstddef>
#include <sstream>
#include <string>


namespace geos {
namespace operation { // geos.operation
namespace geounion {  // geos.operation.geounion


/* public static */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::Union(std::vector<geom::Polygon*>* polys)
{
    CascadedPolygonUnion op(polys);
    return op.Union();
}

/* public static */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::Union(std::vector<geom::Polygon*>* polys, UnionStrategy* unionFun)
{
    CascadedPolygonUnion op(polys, unionFun);
    return op.Union();
}

/* public static */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::Union(const geom::MultiPolygon* multipoly)
{
    std::vector<geom::Polygon*> polys;

    for(const auto& g : *multipoly) {
        polys.push_back(dynamic_cast<geom::Polygon*>(g.get()));
    }

    CascadedPolygonUnion op(&polys);
    return op.Union();
}

/* public */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::Union()
{
    if(inputPolys->empty()) {
        return nullptr;
    }

    geomFactory = inputPolys->front()->getFactory();

    /*
     * A spatial index to organize the collection
     * into groups of close geometries.
     * This makes unioning more efficient, since vertices are more likely
     * to be eliminated on each round.
     */

    index::strtree::TemplateSTRtree<const geom::Geometry*> index(10, inputPolys->size());
    for (const auto& p : *inputPolys) {
        index.insert(p);
    }

    // TODO avoid creating this vector and run binaryUnion off the iterators directly
    std::vector<const geom::Geometry*> geoms(index.items().begin(), index.items().end());

#ifdef GEOS_OPENMP
    return pairUnion(geoms);
#else
    return binaryUnion(geoms, 0, geoms.size());
#endif
}

#ifdef GEOS_OPENMP
#include <omp.h>
#endif

/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::pairUnion(
    std::vector<const geom::Geometry*>& inply) const
{
    int sz = static_cast<int>(inply.size());
    int nsz = (sz / 2) + (sz % 2);
    std::vector<std::unique_ptr<geom::Geometry>> outply(static_cast<std::size_t>(nsz));

#pragma omp parallel for
    for (int i = 0; i < sz; i += 2) {
        /* Work backwards to preserve an old regression result */
        const geom::Geometry* g0 = inply[static_cast<std::size_t>(sz - i - 1)];
        const geom::Geometry* g1 = i+1 < sz ? inply[static_cast<std::size_t>(sz - i - 2)] : nullptr;
        outply[static_cast<std::size_t>(i/2)] = unionSafe(g0, g1);
    }

    int step = 1;
    while(step < nsz) {

#pragma omp parallel for
        /* Place resultant output into position 1 of the inputs */
        /* This keeps threads from stomping on each other as they */
        /* work through different parts of the processing */
        for (int i0 = 0; i0 < sz; i0 += 2*step) {
            int i1 = i0 + step;
            const geom::Geometry* g0 = outply[static_cast<std::size_t>(i0)].release();
            const geom::Geometry* g1 = i1 < sz ? outply[static_cast<std::size_t>(i1)].release() : nullptr;
            outply[static_cast<std::size_t>(i0)] = unionSafe(g0, g1);
        }
        step *= 2;
    }
    return std::move(outply[0]);
}


/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::binaryUnion(
    const std::vector<const geom::Geometry*> & geoms,
    std::size_t start, std::size_t end) const
{
    if(end - start <= 1) {
        return unionSafe(geoms[start], nullptr);
    }
    else if(end - start == 2) {
        return unionSafe(geoms[start], geoms[start + 1]);
    }
    else {
        // recurse on both halves of the list
        std::size_t mid = (end + start) / 2;
        std::unique_ptr<geom::Geometry> g0(binaryUnion(geoms, start, mid));
        std::unique_ptr<geom::Geometry> g1(binaryUnion(geoms, mid, end));
        return unionSafe(std::move(g0), std::move(g1));
    }
}

/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::unionSafe(
    const geom::Geometry* g0, const geom::Geometry* g1) const
{
    if(g0 == nullptr && g1 == nullptr) {
        return nullptr;
    }

    if(g0 == nullptr) {
        return g1->clone();
    }
    if(g1 == nullptr) {
        return g0->clone();
    }

    return unionActual(g0, g1);
}

/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::unionSafe(
    std::unique_ptr<geom::Geometry> && g0,
    std::unique_ptr<geom::Geometry> && g1) const
{
    if(g0 == nullptr && g1 == nullptr) {
        return nullptr;
    }

    if(g0 == nullptr) {
        return std::move(g1);
    }
    if(g1 == nullptr) {
        return std::move(g0);
    }

    return unionActual(std::move(g0), std::move(g1));
}

/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::unionActual(
    const geom::Geometry* g0,
    const geom::Geometry* g1) const
{
    std::unique_ptr<geom::Geometry> ug;
    ug = unionFunction->Union(g0, g1);
    return restrictToPolygons(std::move(ug));
}

/* private */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::unionActual(
    std::unique_ptr<geom::Geometry> && g0,
    std::unique_ptr<geom::Geometry> && g1) const
{
    std::unique_ptr<geom::Geometry> ug;
    ug = unionFunction->Union(std::move(g0), std::move(g1));
    return restrictToPolygons(std::move(ug));
}

/* private static */
std::unique_ptr<geom::Geometry>
CascadedPolygonUnion::restrictToPolygons(std::unique_ptr<geom::Geometry> g)
{
    using namespace geom;

    if(g->isPolygonal()) {
        return g;
    }

    auto gfact = g->getFactory();
    auto coordDim = g->getCoordinateDimension();

    auto coll = dynamic_cast<GeometryCollection*>(g.get());
    if (coll) {
        // Release polygons from the collection and re-form into MultiPolygon
        auto components = coll->releaseGeometries();
        components.erase(std::remove_if(components.begin(), components.end(), [](const std::unique_ptr<Geometry> & cmp) {
            return !cmp->isPolygonal();
        }), components.end());

        return gfact->createMultiPolygon(std::move(components));
    } else {
        // Not polygonal and not a collection? No polygons here.
        return gfact->createPolygon(coordDim);
    }
}

/************************************************************************/

std::unique_ptr<geom::Geometry>
ClassicUnionStrategy::Union(const geom::Geometry* g0, const geom::Geometry* g1)
{
    // TODO make an rvalue overload for this that can consume its inputs.
    // At a minimum, a copy in the buffer fallback can be eliminated.
    try {
        return geom::HeuristicOverlay(g0, g1, operation::overlay::OverlayOp::opUNION);
    }
    catch (const util::TopologyException &ex) {
        ::geos::ignore_unused_variable_warning(ex);
        // union-by-buffer only works for polygons
        if (g0->getDimension() != 2 || g1->getDimension() != 2)
          throw;
        return unionPolygonsByBuffer(g0, g1);
    }
}

bool
ClassicUnionStrategy::isFloatingPrecision() const
{
  return true;
}

/*private*/
std::unique_ptr<geom::Geometry>
ClassicUnionStrategy::unionPolygonsByBuffer(const geom::Geometry* g0, const geom::Geometry* g1)
{
    std::vector<std::unique_ptr<geom::Geometry>> geoms;
    geoms.push_back(g0->clone());
    geoms.push_back(g1->clone());
    std::unique_ptr<geom::GeometryCollection> coll = g0->getFactory()->createGeometryCollection(std::move(geoms));
    return coll->buffer(0);
}





} // namespace geos.operation.union
} // namespace geos.operation
} // namespace geos
