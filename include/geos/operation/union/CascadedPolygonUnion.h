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

#pragma once

#include <geos/export.h>

#include <geos/operation/union/UnionStrategy.h>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class Geometry;
class Polygon;
class MultiPolygon;
class Envelope;
}
}

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion


/**
* \brief
* Implementation of UnionStrategy that provides overlay using
* the first generation overlay routines.
*/
class GEOS_DLL ClassicUnionStrategy : public UnionStrategy {

public:

    ClassicUnionStrategy() {};

    /**
    * Computes the union of two geometries.
    * This method may throw a {@link util::TopologyException}
    * if one is encountered
    */
    std::unique_ptr<geom::Geometry> Union(const geom::Geometry*, const geom::Geometry*) override;

    /**
    * Indicates whether the union function operates using
    * a floating (full) precision model.
    * If this is the case, then the unary union code
    * can make use of the {@link OverlapUnion} performance optimization,
    * and perhaps other optimizations as well.
    * Otherwise, the union result extent may not be the same as the extent of the inputs,
    * which prevents using some optimizations.
    */
    bool isFloatingPrecision() const override;

private:

    /**
    * An alternative way of unioning polygonal geometries
    * by using <code>bufer(0)</code>.
    * Only worth using if regular overlay union fails.
    */
    std::unique_ptr<geom::Geometry> unionPolygonsByBuffer(const geom::Geometry* g0, const geom::Geometry* g1);

};



/**
 * \brief
 * Provides an efficient method of unioning a collection of polygonal geometries.
 *
 * This algorithm is faster and likely more robust than the simple iterated
 * approach of repeatedly unioning each polygon to a result geometry.
 *
 * The `buffer(0)` trick is sometimes faster, but can be less robust and
 * can sometimes take an exceptionally long time to complete.
 * This is particularly the case where there is a high degree of overlap
 * between the polygons.  In this case, `buffer(0)` is forced to compute
 * with *all* line segments from the outset, whereas cascading can eliminate
 * many segments at each stage of processing.
 * The best case for buffer(0) is the trivial case where there is `no` overlap
 * between the input geometries. However, this case is likely rare in practice.
 */
class GEOS_DLL CascadedPolygonUnion {
private:
    std::vector<geom::Polygon*>* inputPolys;
    geom::GeometryFactory const* geomFactory;

    /**
     * The effectiveness of the index is somewhat sensitive
     * to the node capacity.
     * Testing indicates that a smaller capacity is better.
     * For an STRtree, 4 is probably a good number (since
     * this produces 2x2 "squares").
     */
    static int const STRTREE_NODE_CAPACITY = 4;

    /** \brief
     * Computes a [Geometry](@ref geom::Geometry) containing only polygonal components.
     *
     * Extracts the [Polygons](@ref geom::Polygon) from the input
     * and returns them as an appropriate polygonal geometry.
     *
     * If the input is already `Polygonal`, it is returned unchanged.
     *
     * A particular use case is to filter out non-polygonal components
     * returned from an overlay operation.
     *
     * @param g the geometry to filter
     * @return a Polygonal geometry
     */
    static std::unique_ptr<geom::Geometry> restrictToPolygons(std::unique_ptr<geom::Geometry> g);

public:
    CascadedPolygonUnion();

    /** \brief
     * Computes the union of a collection of polygonal [Geometrys](@ref geom::Geometry).
     *
     * @param polys a collection of polygonal [Geometrys](@ref geom::Geometry).
     *              ownership of elements *and* vector are left to caller.
     */
    static std::unique_ptr<geom::Geometry> Union(std::vector<geom::Polygon*>* polys);
    static std::unique_ptr<geom::Geometry> Union(std::vector<geom::Polygon*>* polys, UnionStrategy* unionFun);

    /** \brief
     * Computes the union of a set of polygonal [Geometrys](@ref geom::Geometry).
     *
     * @tparam T an iterator yielding something castable to const Polygon *
     * @param start start iterator
     * @param end end iterator
     * @param unionStrategy strategy to apply
     */
    template <class T>
    static std::unique_ptr<geom::Geometry>
    Union(T start, T end, UnionStrategy *unionStrategy)
    {
        std::vector<geom::Polygon*> polys;
        for(T i = start; i != end; ++i) {
            const geom::Polygon* p = dynamic_cast<const geom::Polygon*>(*i);
            polys.push_back(const_cast<geom::Polygon*>(p));
        }
        return Union(&polys, unionStrategy);
    }

    /** \brief
     * Computes the union of a collection of polygonal [Geometrys](@ref geom::Geometry).
     *
     * @param polys a collection of polygonal [Geometrys](@ref geom::Geometry).
     *              Ownership of elements *and* vector are left to caller.
     */
    static std::unique_ptr<geom::Geometry> Union(const geom::MultiPolygon* polys);

    /** \brief
     * Creates a new instance to union the given collection of
     * [Geometrys](@ref geom::Geometry).
     *
     * @param polys a collection of polygonal [Geometrys](@ref geom::Geometry).
     *              Ownership of elements *and* vector are left to caller.
     */
    CascadedPolygonUnion(std::vector<geom::Polygon*>* polys)
        : inputPolys(polys)
        , geomFactory(nullptr)
        , unionFunction(&defaultUnionFunction)
    {}

    CascadedPolygonUnion(std::vector<geom::Polygon*>* polys, UnionStrategy* unionFun)
        : inputPolys(polys)
        , geomFactory(nullptr)
        , unionFunction(unionFun)
    {}

    /** \brief
     * Computes the union of the input geometries.
     *
     * @return the union of the input geometries
     * @return `null` if no input geometries were provided
     */
    std::unique_ptr<geom::Geometry> Union();

private:

    UnionStrategy* unionFunction;
    ClassicUnionStrategy defaultUnionFunction;

    /**
     * Unions a section of a list using a recursive binary union on each half
     * of the section.
     *
     * @param geoms the list of geometries containing the section to union
     * @param start the start index of the section
     * @param end the index after the end of the section
     * @return the union of the list section
     */
    std::unique_ptr<geom::Geometry> binaryUnion(const std::vector<const geom::Geometry*> & geoms, std::size_t start, std::size_t end);

    /**
     * Computes the union of two geometries,
     * either of both of which may be null.
     *
     * @param g0 a Geometry
     * @param g1 a Geometry
     * @return the union of the input(s)
     * @return null if both inputs are null
     */
    std::unique_ptr<geom::Geometry> unionSafe(const geom::Geometry* g0, const geom::Geometry* g1) const;

    std::unique_ptr<geom::Geometry> unionSafe(std::unique_ptr<geom::Geometry> &&, std::unique_ptr<geom::Geometry> &&);

    /**
     * Encapsulates the actual unioning of two polygonal geometries.
     *
     * @param g0
     * @param g1
     * @return
     */
    std::unique_ptr<geom::Geometry> unionActual(const geom::Geometry* g0, const geom::Geometry* g1) const;

    std::unique_ptr<geom::Geometry> unionActual(std::unique_ptr<geom::Geometry> &&, std::unique_ptr<geom::Geometry> &&) const;
};





} // namespace geos::operation::union
} // namespace geos::operation
} // namespace geos

