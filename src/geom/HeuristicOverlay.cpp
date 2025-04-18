/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2013-2020 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************
 *
 * This file provides a single function, taking two
 * const Geometry pointers, applying a binary operator to them
 * and returning a result Geometry in an unique_ptr<>.
 * 
 * It implements a "combine-disjoint" heuristic for optimizing some overlay cases.
 * It also implements overlay for GeometryCollections, which is not (yet)
 * provided by OverlayNG.
 * Internal overlay usage should call OverlayNG directly 
 * if this behaviour is not needed (i.e. when the inputs are known to be simple
 * or when full overlay is desired to be computed.)
 *
 **********************************************************************/

#include <geos/geom/HeuristicOverlay.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/overlayng/OverlayUtil.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/io/WKTWriter.h>

namespace geos {
namespace geom { // geos::geom

using operation::overlayng::OverlayNG;
using operation::overlayng::OverlayNGRobust;
using operation::overlayng::OverlayUtil;

bool
hasSingleNonEmptyElement(const Geometry* geom) {
    if (geom->getGeometryTypeId() != GEOS_GEOMETRYCOLLECTION) {
        //-- is a non-empty element 
        return ! geom->isEmpty();
    }
    //-- iterate over GC elements and determine if a single non-empty is present
    //-- buffer num geoms, since can be expensive for nested collections
    bool foundNonEmptyElement = false;
    std::size_t numGeoms = geom->getNumGeometries();
    for(std::size_t i = 0; i < numGeoms; ++i) {
        if (hasSingleNonEmptyElement(geom->getGeometryN(i))) {
            //-- already found an element, so more than one present
            if (foundNonEmptyElement) {
                return false;
            }
            foundNonEmptyElement = true;
        }
    }
    return foundNonEmptyElement;
}

bool isCombinable(const Geometry* g0, const Geometry* g1)
{
    //-- if both empty use OverlayNG return-type logic
    if (g0->isEmpty() && g1->isEmpty())
        return false;
    
    //-- not disjoint
    if (g0->getEnvelopeInternal()->intersects(g1->getEnvelopeInternal()))
        return false;

    return hasSingleNonEmptyElement(g0) 
        && hasSingleNonEmptyElement(g1);
}

void 
extractElements(const Geometry* g, std::vector<std::unique_ptr<Geometry>>& v)
{
    if (const auto* coll = dynamic_cast<const GeometryCollection*>(g)) {
        //-- buffer num geoms, since can be expensive for nested collections
        std::size_t numGeoms = g->getNumGeometries();
        for(std::size_t i = 0; i < numGeoms; ++i) {
            //-- recurse to handle nested GCs
            extractElements(coll->getGeometryN(i), v);
        }
    }
    else if (g->isEmpty()) {
        return;
    }
    else {
        v.push_back(g->clone());
    }
}

std::unique_ptr<Geometry>
combineReduced(const Geometry* g0, const Geometry* g1)
{
    // Allocated for ownership transfer
    std::vector<std::unique_ptr<Geometry>> v;
    v.reserve(g0->getNumGeometries() + g1->getNumGeometries());
    extractElements(g0, v);
    extractElements(g1, v);
    return g0->getFactory()->buildGeometry(std::move(v));
}

bool isHandledByOverlayNG(const Geometry* geom) {
    if (geom->isMixedDimension() && ! geom->isEmpty())
        return false;
    //-- GCs with polygonals must be unioned 
    if (geom->getGeometryTypeId() == GEOS_GEOMETRYCOLLECTION 
        && geom->getDimension() == 2)
        return false;
    return true;
}

std::unique_ptr<Geometry>
HeuristicOverlay(const Geometry* g0, const Geometry* g1, int opCode)
{
    /**
     * If feasible, do fast combine instead of full overlay 
     * 
     * NOTE: does not node LineStrings, or merge elements of arg geoms.
     * This is different behaviour to full overlay.
     */
    //TODO: could also handle difference and intersection?
    if ( ( opCode == OverlayNG::UNION 
        || opCode == OverlayNG::SYMDIFFERENCE)
            && isCombinable(g0, g1)) {
        return combineReduced(g0, g1);
    }

    /*
    * overlayng::OverlayNGRobust does not currently handle
    * non-empty GeometryCollections
    * so we handle that case here.
    */
    if (! isHandledByOverlayNG(g0) ||
        ! isHandledByOverlayNG(g1))
    {
        return StructuredCollection::overlay(g0, g1, opCode);
    }

    /*
    * overlayng::OverlayNGRobust carries out the following steps
    *
    * 1. Perform overlay operation using PrecisionModel(float).
    *    If no exception return result.
    * 2. Perform overlay operation using SnappingNoder(tolerance), starting
    *    with a very very small tolerance and increasing it for 5 iterations.
    *    The SnappingNoder moves only nodes that are within tolerance of
    *    other nodes and lines, leaving all the rest undisturbed, for a very
    *    clean result, if it manages to create one.
    *    If a result is found with no exception, return.
    * 3. Perform overlay operation using a PrecisionModel(scale), which
    *    uses a SnapRoundingNoder. Every vertex will be noded to the snapping
    *    grid, resulting in a modified geometry. The SnapRoundingNoder approach
    *    reliably produces results, assuming valid inputs.
    *
    * Running overlayng::OverlayNGRobust at this stage should guarantee
    * that none of the other heuristics are ever needed.
    */
    std::unique_ptr<Geometry> ret;
    if (g0 == nullptr && g1 == nullptr) {
        return std::unique_ptr<Geometry>(nullptr);
    }
    else if (g0 == nullptr) {
        // Use a unary union for the one-parameter case, as the pairwise
        // union with one parameter is very intolerant to invalid
        // collections and multi-polygons.
        ret = OverlayNGRobust::Union(g1);
    }
    else if (g1 == nullptr) {
        // Use a unary union for the one-parameter case, as the pairwise
        // union with one parameter is very intolerant to invalid
        // collections and multi-polygons.
        ret = OverlayNGRobust::Union(g0);
    }
    else {
        ret = OverlayNGRobust::Overlay(g0, g1, opCode);
    }

    return ret;
}

/* public static */
std::unique_ptr<Geometry>
StructuredCollection::overlay(const Geometry* g0, const Geometry* g1, int opCode)
{
    StructuredCollection s0(g0);
    StructuredCollection s1(g1);
    switch (opCode) {
    case OverlayNG::UNION:
        return s0.doUnion(s1);
    case OverlayNG::DIFFERENCE:
        return s0.doDifference(s1);
    case OverlayNG::SYMDIFFERENCE:
        return s0.doSymDifference(s1);
    case OverlayNG::INTERSECTION:
        return s0.doIntersection(s1);    
    default:
        throw util::IllegalArgumentException("Invalid overlay opcode");
    }
}

/* private */
void 
StructuredCollection::addDimension(Dimension::DimensionType dim)
{
    if (dimension < dim)
        dimension = dim;
}

/* public */
void
StructuredCollection::readCollection(const Geometry* g)
{
    if (!factory) factory = g->getFactory();
    if (g->isCollection()) {
        for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
            readCollection(g->getGeometryN(i));
        }
    }
    else {
        if (g->isEmpty()) return;
        switch (g->getGeometryTypeId()) {
            case GEOS_POINT:
                pts.push_back(g);
                addDimension(Dimension::P);
                break;
            case GEOS_LINESTRING:
                lines.push_back(g);
                addDimension(Dimension::L);
                break;
            case GEOS_POLYGON:
                polys.push_back(g);
                addDimension(Dimension::A);
                break;
            default:
                throw util::IllegalArgumentException("cannot process unexpected collection");
        }
    }
}

/* public static */
void
StructuredCollection::toVector(const Geometry* g, std::vector<const Geometry*>& v)
{
    if (!g || g->isEmpty()) return;
    if (g->isCollection()) {
        for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
            toVector(g->getGeometryN(i), v);
        }
    }
    else {
        switch (g->getGeometryTypeId()) {
            case GEOS_POINT:
            case GEOS_LINESTRING:
            case GEOS_POLYGON:
                v.push_back(g);
                break;
            default:
                return;
        }
    }
}


/* public */
void
StructuredCollection::unionByDimension(void)
{
    /*
    * Remove duplication within each dimension, so that there
    * is only one object covering any particular space within
    * that dimension.
    * This makes reasoning about the collection-on-collection
    * operations a little easier later on.
    */
    std::unique_ptr<MultiPoint> pt_col = factory->createMultiPoint(pts);
    std::unique_ptr<MultiLineString> line_col = factory->createMultiLineString(lines);
    std::unique_ptr<MultiPolygon> poly_col = factory->createMultiPolygon(polys);

    pt_union = OverlayNGRobust::Union(static_cast<const Geometry*>(pt_col.get()));
    line_union = OverlayNGRobust::Union(static_cast<const Geometry*>(line_col.get()));
    poly_union = OverlayNGRobust::Union(static_cast<const Geometry*>(poly_col.get()));

    // io::WKTWriter w;
    // std::cout << "line_col " << w.write(*line_col) << std::endl;
    // std::cout << "pt_union: " << w.write(*pt_union) << std::endl;
    // std::cout << "line_union: " << w.write(*line_union) << std::endl;
    // std::cout << "poly_union: " << w.write(*poly_union) << std::endl;

    if (! pt_union->isPuntal())
        throw util::IllegalArgumentException("union of points not puntal");
    if (! line_union->isLineal())
        throw util::IllegalArgumentException("union of lines not lineal");
    if (! poly_union->isPolygonal())
        throw util::IllegalArgumentException("union of polygons not polygonal");
}

/* public */
std::unique_ptr<Geometry>
StructuredCollection::doUnaryUnion(int resultDim) const
{
    /*
    * Before output, we clean up the components to remove spatial
    * duplication. Points that lines pass through. Lines that are covered
    * by polygonal areas already. Provides a "neater" output that still
    * covers all the area it should.
    */
    std::unique_ptr<Geometry> pts_less_lines = OverlayNGRobust::Overlay(
        pt_union.get(),
        line_union.get(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> pts_less_polys_lines = OverlayNGRobust::Overlay(
        pts_less_lines.get(),
        poly_union.get(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> lines_less_polys = OverlayNGRobust::Overlay(
        line_union.get(),
        poly_union.get(),
        OverlayNG::DIFFERENCE);

    std::vector<const Geometry*> geoms;
    toVector(pts_less_polys_lines.get(), geoms);
    toVector(lines_less_polys.get(), geoms);
    toVector(poly_union.get(), geoms);

    if (geoms.size() == 0) {
        return OverlayUtil::createEmptyResult(
            resultDim, 2, factory);
    }
    return factory->buildGeometry(geoms.begin(), geoms.end());
}

/* public */
std::unique_ptr<Geometry>
StructuredCollection::computeResult(StructuredCollection& coll, int opCode,
    Dimension::DimensionType dimA, Dimension::DimensionType dimB) const
{
    coll.unionByDimension();
    int resultDim = OverlayUtil::resultDimension(opCode, dimA, dimB);
    return coll.doUnaryUnion(resultDim);
}

/* public */
std::unique_ptr<Geometry>
StructuredCollection::doUnion(const StructuredCollection& a) const
{
    auto poly_union_poly = OverlayNGRobust::Overlay(
        a.getPolyUnion(),
        poly_union.get(),
        OverlayNG::UNION);

    auto line_union_line = OverlayNGRobust::Overlay(
        a.getLineUnion(),
        line_union.get(),
        OverlayNG::UNION);

    auto pt_union_pt = OverlayNGRobust::Overlay(
        a.getPointUnion(),
        pt_union.get(),
        OverlayNG::UNION);

    StructuredCollection c;
    c.readCollection(poly_union_poly.get());
    c.readCollection(line_union_line.get());
    c.readCollection(pt_union_pt.get());
    return computeResult(c, OverlayNG::UNION, getDimension(), a.getDimension());
}


std::unique_ptr<Geometry>
StructuredCollection::doIntersection(const StructuredCollection& a) const
{
    std::unique_ptr<Geometry> poly_inter_poly = OverlayNGRobust::Overlay(
        poly_union.get(),
        a.getPolyUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> poly_inter_line = OverlayNGRobust::Overlay(
        poly_union.get(),
        a.getLineUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> poly_inter_pt = OverlayNGRobust::Overlay(
        poly_union.get(),
        a.getPointUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> line_inter_poly = OverlayNGRobust::Overlay(
        line_union.get(),
        a.getPolyUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> line_inter_line = OverlayNGRobust::Overlay(
        line_union.get(),
        a.getLineUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> line_inter_pt = OverlayNGRobust::Overlay(
        line_union.get(),
        a.getPointUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> pt_inter_pt = OverlayNGRobust::Overlay(
        pt_union.get(),
        a.getPointUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> pt_inter_line = OverlayNGRobust::Overlay(
        pt_union.get(),
        a.getLineUnion(),
        OverlayNG::INTERSECTION);

    std::unique_ptr<Geometry> pt_inter_poly = OverlayNGRobust::Overlay(
        pt_union.get(),
        a.getPolyUnion(),
        OverlayNG::INTERSECTION);

    // io::WKTWriter w;
    // std::cout << "poly_inter_poly " << w.write(*poly_inter_poly) << std::endl;
    // std::cout << "poly_union.get() " << w.write(poly_union.get()) << std::endl;
    // std::cout << "a.getLineUnion() " << w.write(a.getLineUnion()) << std::endl;
    // std::cout << "poly_inter_line " << w.write(*poly_inter_line) << std::endl;
    // std::cout << "poly_inter_pt " << w.write(*poly_inter_pt) << std::endl;
    // std::cout << "line_inter_line " << w.write(*line_inter_line) << std::endl;
    // std::cout << "line_inter_pt " << w.write(*line_inter_pt) << std::endl;
    // std::cout << "pt_inter_pt " << w.write(*pt_inter_pt) << std::endl;

    StructuredCollection c;
    c.readCollection(poly_inter_poly.get());
    c.readCollection(poly_inter_line.get());
    c.readCollection(poly_inter_pt.get());
    c.readCollection(line_inter_poly.get());
    c.readCollection(line_inter_line.get());
    c.readCollection(line_inter_pt.get());
    c.readCollection(pt_inter_poly.get());
    c.readCollection(pt_inter_line.get());
    c.readCollection(pt_inter_pt.get());
    return computeResult(c, OverlayNG::INTERSECTION, getDimension(), a.getDimension());
}


std::unique_ptr<Geometry>
StructuredCollection::doDifference(const StructuredCollection& a) const
{
    std::unique_ptr<Geometry> poly_diff_poly = OverlayNGRobust::Overlay(
        poly_union.get(),
        a.getPolyUnion(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> line_diff_poly = OverlayNGRobust::Overlay(
        line_union.get(),
        a.getPolyUnion(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> pt_diff_poly = OverlayNGRobust::Overlay(
        pt_union.get(),
        a.getPolyUnion(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> line_diff_poly_line = OverlayNGRobust::Overlay(
        line_diff_poly.get(),
        a.getLineUnion(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> pt_diff_poly_line = OverlayNGRobust::Overlay(
        pt_diff_poly.get(),
        line_diff_poly_line.get(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> pt_diff_poly_line_pt = OverlayNGRobust::Overlay(
        pt_diff_poly_line.get(),
        a.getPointUnion(),
        OverlayNG::DIFFERENCE);

    StructuredCollection c;
    c.readCollection(poly_diff_poly.get());
    c.readCollection(line_diff_poly_line.get());
    c.readCollection(pt_diff_poly_line_pt.get());
    return computeResult(c, OverlayNG::DIFFERENCE, getDimension(), a.getDimension());
}

std::unique_ptr<Geometry>
StructuredCollection::doSymDifference(const StructuredCollection& a) const
{
    std::unique_ptr<Geometry> poly_symdiff_poly = OverlayNGRobust::Overlay(
        poly_union.get(),
        a.getPolyUnion(),
        OverlayNG::SYMDIFFERENCE);

    std::unique_ptr<Geometry> line_symdiff_line = OverlayNGRobust::Overlay(
        line_union.get(),
        a.getLineUnion(),
        OverlayNG::DIFFERENCE);

    std::unique_ptr<Geometry> pt_symdiff_pt = OverlayNGRobust::Overlay(
        pt_union.get(),
        a.getPointUnion(),
        OverlayNG::DIFFERENCE);

    StructuredCollection c;
    c.readCollection(poly_symdiff_poly.get());
    c.readCollection(line_symdiff_line.get());
    c.readCollection(pt_symdiff_pt.get());
    return computeResult(c, OverlayNG::SYMDIFFERENCE, getDimension(), a.getDimension());
}


} // namespace geos::geom
} // namespace geos
