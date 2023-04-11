
/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2023 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <vector>
#include <memory>
#include <geos/export.h>


namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
class MultiLineString;
}
namespace coverage {
class CoverageEdge;
}
}


using geos::coverage::CoverageEdge;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::MultiLineString;


namespace geos {
namespace coverage { // geos::coverage

/**
 * Simplifies the boundaries of the polygons in a polygonal coverage
 * while preserving the original coverage topology.
 * An area-based simplification algorithm
 * (similar to Visvalingam-Whyatt simplification)
 * is used to provide high-quality results.
 * Also supports simplifying just the inner edges in a coverage,
 * which allows simplifying "patches" without affecting their boundary.
 *
 * The amount of simplification is determined by a tolerance value,
 * which is a non-negative quantity. It equates roughly to the maximum
 * distance by which a simplified line can change from the original.
 * (In fact, it is the square root of the area tolerance used
 * in the Visvalingam-Whyatt algorithm.)
 *
 * The simplified result coverage has the following characteristics:
 *
 *   * It has the same number and types of polygonal geometries as the input
 *   * Node points (inner vertices shared by three or more polygons,
 *     or boundary vertices shared by two or more) are not changed
 *   * If the input is a valid coverage, then so is the result
 *
 * This class also supports inner simplification, which simplifies
 * only edges of the coverage which are adjacent to two polygons.
 * This allows partial simplification of a coverage, since a simplified
 * subset of a coverage still matches the remainder of the coverage.
 *
 * The input coverage should be valid according to {@link CoverageValidator}.
 *
 * @author Martin Davis
 */
class GEOS_DLL CoverageSimplifier {


public:

    /**
    * Create a new coverage simplifier instance.
    *
    * @param coverage a set of polygonal geometries forming a coverage
    */
    CoverageSimplifier(std::vector<const Geometry*>& coverage);

    /**
    * Simplifies the boundaries of a set of polygonal geometries forming a coverage,
    * preserving the coverage topology.
    *
    * @param coverage a set of polygonal geometries forming a coverage
    * @param tolerance the simplification tolerance
    * @return the simplified polygons
    */
    static std::vector<std::unique_ptr<Geometry>> simplify(
        std::vector<const Geometry*>& coverage,
        double tolerance);

    static std::vector<std::unique_ptr<Geometry>> simplify(
        const std::vector<std::unique_ptr<Geometry>>& coverage,
        double tolerance);

    /**
    * Simplifies the inner boundaries of a set of polygonal geometries forming a coverage,
    * preserving the coverage topology.
    * Edges which form the exterior boundary of the coverage are left unchanged.
    *
    * @param coverage a set of polygonal geometries forming a coverage
    * @param tolerance the simplification tolerance
    * @return the simplified polygons
    */
    static std::vector<std::unique_ptr<Geometry>> simplifyInner(
        std::vector<const Geometry*>& coverage,
        double tolerance);

    static std::vector<std::unique_ptr<Geometry>> simplifyInner(
        const std::vector<std::unique_ptr<Geometry>>& coverage,
        double tolerance);

    /**
    * Computes the simplified coverage, preserving the coverage topology.
    *
    * @param tolerance the simplification tolerance
    * @return the simplified polygons
    */
    std::vector<std::unique_ptr<Geometry>> simplify(
        double tolerance);

    /**
    * Computes the inner-boundary simplified coverage,
    * preserving the coverage topology,
    * and leaving outer boundary edges unchanged.
    *
    * @param tolerance the simplification tolerance
    * @return the simplified polygons
    */
    std::vector<std::unique_ptr<Geometry>> simplifyInner(
        double tolerance);


private:

    // Members
    std::vector<const Geometry*>& m_input; // TODO? make this const
    const GeometryFactory* m_geomFactory;

    // Methods
    void simplifyEdges(
        std::vector<CoverageEdge*> edges,
        const MultiLineString* constraints,
        double tolerance);

    void setCoordinates(
        std::vector<CoverageEdge*>& edges,
        const MultiLineString* lines);

    std::vector<bool> getFreeRings(
        const std::vector<CoverageEdge*>& edges) const;


}; // CoverageSimplifier


} // geos::coverage
} // geos
