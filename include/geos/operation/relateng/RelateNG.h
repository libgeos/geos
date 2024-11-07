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

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <string>
#include <sstream>


// Forward declarations
namespace geos {
namespace algorithm {
    class BoundaryNodeRule;

}
namespace geom {
    class Geometry;
}
namespace noding {
}
namespace operation {
namespace relateng {
    class TopologyPredicate;
    class TopologyComputer;
    class EdgeSegmentIntersector;
}
}
}


namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

/**
 * Computes the value of topological predicates between two geometries based on the
 * Dimensionally-Extended 9-Intersection Model <https://en.wikipedia.org/wiki/DE-9IM> (DE-9IM).
 * Standard and custom topological predicates are provided by RelatePredicate.
 *
 * The RelateNG algorithm has the following capabilities:
 *
 *   * Efficient short-circuited evaluation of topological predicates
 *     (including matching custom DE-9IM matrix patterns)
 *   * Optimized repeated evaluation of predicates against a single geometry
 *     via cached spatial indexes (AKA "prepared mode")
 *   * Robust computation (only point-local topology is required,
 *     so invalid geometry topology does not cause failures)
 *   * GeometryCollection inputs containing mixed types and overlapping polygons
 *     are supported, using union semantics.
 *   * Zero-length LineStrings are treated as being topologically identical to Points.
 *   * Support for BoundaryNodeRule.
 *
 * See IntersectionMatrixPattern for a description of DE-9IM patterns.
 *
 * If not specified, the standard BoundaryNodeRule::MOD2_BOUNDARY_RULE is used.
 *
 * RelateNG operates in 2D only; it ignores any Z ordinates.
 *
 * This implementation replaces RelateOp and PreparedGeometry.
 *
 * FUTURE WORK
 *
 *   * Support for a distance tolerance to provide "approximate" predicate evaluation
 *
 * @author Martin Davis
 *
 * @see RelateOp
 * @see PreparedGeometry
 */
class GEOS_DLL RelateNG {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;
    using BoundaryNodeRule = geos::algorithm::BoundaryNodeRule;
    using MCIndexSegmentSetMutualIntersector = geos::noding::MCIndexSegmentSetMutualIntersector;
    using LinearRing = geos::geom::LinearRing;
    using Envelope = geos::geom::Envelope;
    using SegmentString = geos::noding::SegmentString;
    using IntersectionMatrix = geos::geom::IntersectionMatrix;

private:

    // Members
    const BoundaryNodeRule& boundaryNodeRule;
    RelateGeometry geomA;
    std::unique_ptr<MCIndexSegmentSetMutualIntersector> edgeMutualInt = nullptr;

    // Methods

    RelateNG(const Geometry* inputA, bool isPrepared, const BoundaryNodeRule& bnRule)
        : boundaryNodeRule(bnRule)
        , geomA(inputA, isPrepared, bnRule)
        {}

    RelateNG(const Geometry* inputA, bool isPrepared)
        : RelateNG(inputA, isPrepared, BoundaryNodeRule::getBoundaryRuleMod2())
        {}

    bool hasRequiredEnvelopeInteraction(const Geometry* b, TopologyPredicate& predicate);

    bool finishValue(TopologyPredicate& predicate);

    void computePP(RelateGeometry& geomB, TopologyComputer& topoComputer);

    void computeAtPoints(RelateGeometry& geom, bool isA, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    bool computePoints(RelateGeometry& geom, bool isA, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    void computePoint(bool isA, const CoordinateXY* pt, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    bool computeLineEnds(RelateGeometry& geom, bool isA, RelateGeometry& geomTarget, TopologyComputer& topoComputer);


    /**
     * Compute the topology of a line endpoint.
     * Also reports if the line end is in the exterior of the target geometry,
     * to optimize testing multiple exterior endpoints.
     *
     * @param geom
     * @param isA
     * @param pt
     * @param geomTarget
     * @param topoComputer
     * @return true if the line endpoint is in the exterior of the target
     */
    bool computeLineEnd(RelateGeometry& geom, bool isA, const CoordinateXY* pt, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    bool computeAreaVertex(RelateGeometry& geom, bool isA, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    bool computeAreaVertex(RelateGeometry& geom, bool isA, const LinearRing* ring, RelateGeometry& geomTarget, TopologyComputer& topoComputer);

    void computeAtEdges(RelateGeometry& geomB, TopologyComputer& topoComputer);

    void computeEdgesAll(std::vector<const SegmentString*>& edgesB, const Envelope* envInt, EdgeSegmentIntersector& intersector);

    void computeEdgesMutual(std::vector<const SegmentString*>& edgesB, const Envelope* envInt, EdgeSegmentIntersector& intersector);



public:

    /**
     * Tests whether the topological relationship between two geometries
     * satisfies a topological predicate.
     *
     * @param a the A input geometry
     * @param b the B input geometry
     * @param pred the topological predicate
     * @return true if the topological relationship is satisfied
     */
    static bool relate(const Geometry* a, const Geometry* b, TopologyPredicate& pred);

    /**
     * Tests whether the topological relationship between two geometries
     * satisfies a topological predicate,
     * using a given BoundaryNodeRule.
     *
     * @param a the A input geometry
     * @param b the B input geometry
     * @param pred the topological predicate
     * @param bnRule the Boundary Node Rule to use
     * @return true if the topological relationship is satisfied
     */
    static bool relate(const Geometry* a, const Geometry* b, TopologyPredicate& pred, const BoundaryNodeRule& bnRule);

    /**
     * Tests whether the topological relationship to a geometry
     * matches a DE-9IM matrix pattern.
     *
     * @param a the A input geometry
     * @param b the B input geometry
     * @param imPattern the DE-9IM pattern to match
     * @return true if the geometries relationship matches the DE-9IM pattern
     *
     * @see IntersectionMatrixPattern
     */
    static bool relate(const Geometry* a, const Geometry* b, const std::string& imPattern);

    /**
     * Computes the DE-9IM matrix
     * for the topological relationship between two geometries.
     *
     * @param a the A input geometry
     * @param b the B input geometry
     * @return the DE-9IM matrix for the topological relationship
     */
    static std::unique_ptr<IntersectionMatrix> relate(const Geometry* a, const Geometry* b);

    /**
     * Computes the DE-9IM matrix
     * for the topological relationship between two geometries.
     *
     * @param a the A input geometry
     * @param b the B input geometry
     * @param bnRule the Boundary Node Rule to use
     * @return the DE-9IM matrix for the relationship
     */
    static std::unique_ptr<IntersectionMatrix> relate(const Geometry* a, const Geometry* b, const BoundaryNodeRule& bnRule);

    /**
     * Creates a prepared RelateNG instance to optimize the
     * evaluation of relationships against a single geometry.
     *
     * @param a the A input geometry
     * @return a prepared instance
     */
    static std::unique_ptr<RelateNG> prepare(const Geometry* a);

    /**
     * Creates a prepared RelateNG instance to optimize the
     * computation of predicates against a single geometry,
     * using a given BoundaryNodeRule.
     *
     * @param a the A input geometry
     * @param bnRule the required BoundaryNodeRule
     * @return a prepared instance
     */
    static std::unique_ptr<RelateNG> prepare(const Geometry* a, const BoundaryNodeRule& bnRule);


    /**
     * Computes the DE-9IM matrix for the topological relationship to a geometry.
     *
     * @param b the B geometry to test against
     * @return the DE-9IM matrix
     */
    std::unique_ptr<IntersectionMatrix> evaluate(const Geometry* b);


    /**
     * Tests whether the topological relationship to a geometry
     * matches a DE-9IM matrix pattern.
     *
     * @param b the B geometry to test against
     * @param imPattern the DE-9IM pattern to match
     * @return true if the geometries' topological relationship matches the DE-9IM pattern
     *
     * @see IntersectionMatrixPattern
     */
    bool evaluate(const Geometry* b, const std::string& imPattern);

    /**
     * Tests whether the topological relationship to a geometry
     * satisfies a topology predicate.
     *
     * @param b the B geometry to test against
     * @param predicate the topological predicate
     * @return true if the predicate is satisfied
     */
    bool evaluate(const Geometry* b, TopologyPredicate& predicate);

    static bool intersects(const Geometry* a, const Geometry* b);
    static bool crosses(const Geometry* a, const Geometry* b);
    static bool disjoint(const Geometry* a, const Geometry* b);
    static bool touches(const Geometry* a, const Geometry* b);
    static bool within(const Geometry* a, const Geometry* b);
    static bool contains(const Geometry* a, const Geometry* b);
    static bool overlaps(const Geometry* a, const Geometry* b);
    static bool covers(const Geometry* a, const Geometry* b);
    static bool coveredBy(const Geometry* a, const Geometry* b);
    static bool equalsTopo(const Geometry* a, const Geometry* b);

    bool intersects(const Geometry* a);
    bool crosses(const Geometry* a);
    bool disjoint(const Geometry* a);
    bool touches(const Geometry* a);
    bool within(const Geometry* a);
    bool contains(const Geometry* a);
    bool overlaps(const Geometry* a);
    bool covers(const Geometry* a);
    bool coveredBy(const Geometry* a);
    bool equalsTopo(const Geometry* a);
    bool relate(const Geometry* a, const std::string& pat);
    std::unique_ptr<IntersectionMatrix> relate(const Geometry* a);

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

