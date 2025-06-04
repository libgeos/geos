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

#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/coverage/CoverageEdge.h> // to materialize CoverageEdge
#include <geos/util/Progress.h>

#include <set>
#include <map>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class Geometry;
class LinearRing;
class MultiPolygon;
class Polygon;
}
namespace coverage {
class CoverageEdge;
}
}

namespace geos {     // geos
namespace coverage { // geos.coverage

/**
 * Models a polygonal coverage as a set of unique {@link CoverageEdge}s,
 * linked to the parent rings in the coverage polygons.
 * Each edge has either one or two parent rings, depending on whether
 * it is an inner or outer edge of the coverage.
 * The source coverage is represented as a array of polygonal geometries
 * (either {@link geos::geom::Polygon}s or {@link geos::geom::MultiPolygon}s).
 *
 * @author Martin Davis
 */
class GEOS_DLL CoverageRingEdges {
    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using LinearRing = geos::geom::LinearRing;
    using LineSegment = geos::geom::LineSegment;
    using MultiPolygon = geos::geom::MultiPolygon;
    using Polygon = geos::geom::Polygon;

private:

    // Members
    const std::vector<const Geometry*>& m_coverage;
    std::map<const LinearRing*, std::vector<CoverageEdge*>> m_ringEdgesMap;
    std::vector<CoverageEdge*> m_edges;
    std::vector<std::unique_ptr<CoverageEdge>> m_edgeStore;

    /* Turn off copy constructors for MSVC */
    CoverageRingEdges(const CoverageRingEdges&) = delete;
    CoverageRingEdges& operator=(const CoverageRingEdges&) = delete;

public:

    CoverageRingEdges(const std::vector<const Geometry*>& coverage,
                      geos::util::ProgressFunction* progressFunction)
        : m_coverage(coverage)
    {
        build(progressFunction);
    };


    std::vector<CoverageEdge*>& getEdges()
    {
        return m_edges;
    };

    /**
    * Selects the edges with a given ring count (which can be 1 or 2).
    *
    * @param ringCount the edge ring count to select (1 or 2)
    * @return the selected edges
    */
    std::vector<CoverageEdge*> selectEdges(
        std::size_t ringCount) const;

    /**
    * Recreates the polygon coverage from the current edge values.
    *
    * @param progressFunction Progress function or null
    * @return an array of polygonal geometries representing the coverage
    */
    std::vector<std::unique_ptr<Geometry>> buildCoverage(geos::util::ProgressFunction* progressFunction) const;


private:

    void build(geos::util::ProgressFunction* progressFunction);

    void addRingEdges(
        const LinearRing* ring,
        Coordinate::UnorderedSet& nodes,
        LineSegment::UnorderedSet& boundarySegs,
        std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap);

    void addBoundaryInnerNodes(
        const LinearRing* ring,
        LineSegment::UnorderedSet& boundarySegs,
        Coordinate::UnorderedSet& nodes);

    std::vector<CoverageEdge*> extractRingEdges(
        const LinearRing* ring,
        std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap,
        Coordinate::UnorderedSet& nodes);

    CoverageEdge* createEdge(
        const CoordinateSequence& ring,
        std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap);

    CoverageEdge* createEdge(
        const CoordinateSequence& ring,
        std::size_t start, std::size_t end,
        std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap);

    std::size_t findNextNodeIndex(
        const CoordinateSequence& ring,
        std::size_t start,
        Coordinate::UnorderedSet& nodes) const;

    static std::size_t next(
        std::size_t index,
        const CoordinateSequence& ring);

    Coordinate::UnorderedSet findMultiRingNodes(
        const std::vector<const Geometry*>& coverage,
        geos::util::ProgressFunction* progressFunction);

    Coordinate::UnorderedSet findBoundaryNodes(
        LineSegment::UnorderedSet& lineSegments,
        geos::util::ProgressFunction* progressFunction);

    std::unique_ptr<Geometry> buildPolygonal(
        const Geometry* geom) const;

    std::unique_ptr<Geometry> buildMultiPolygon(
        const MultiPolygon* geom) const;

    std::unique_ptr<Polygon> buildPolygon(
        const Polygon* polygon) const;

    std::unique_ptr<LinearRing> buildRing(
        const LinearRing* ring) const;

    bool isEdgeDirForward(
        const std::vector<CoverageEdge*>& ringEdges,
        std::size_t index,
        const Coordinate& prevPt) const;


};

} // namespace geos.coverage
} // namespace geos
