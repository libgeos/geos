/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
}
namespace noding {
class NodedSegmentString;
class SegmentString;
}
}

namespace geos {
namespace operation {
namespace linenode {

/**
 * Nodes a collection of linear geometries against each other,
 * returning a collection of the same size where each member has
 * been split into a MultiLineString at all interior node points.
 *
 * Unlike geos::noding::GeometryNoder::node(), which collects all
 * input edges into a single flattened MultiLineString, this class
 * preserves the identity of each input member. Linework that is
 * shared (or nearly shared) between members is not dissolved.
 *
 * Input geometries must be linear (LineString, MultiLineString, or
 * a GeometryCollection of linear types). Non-linear components (e.g.
 * polygon rings, points) are silently ignored; their output slot
 * will contain an empty MultiLineString.
 *
 * When gridSize == 0.0 (default), standard IteratedNoder is used.
 * When gridSize > 0.0, SnapRoundingNoder is used instead, providing
 * robust output for inputs with near-coincident coordinates
 * (see https://github.com/libgeos/geos/issues/877).
 * Values <= 0.0 all use IteratedNoder.
 *
 * Usage:
 * @code
 *   std::vector<const Geometry*> lines = { ... };
 *   auto noded = LineCollectionNoder::node(lines);        // standard noding
 *   auto noded = LineCollectionNoder::node(lines, 1.0);   // snap-rounding
 * @endcode
 *
 * @author Paul Ramsey
 */
class GEOS_DLL LineCollectionNoder {

    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using NodedSegmentString = geos::noding::NodedSegmentString;
    using SegmentString = geos::noding::SegmentString;

public:

    /**
     * Creates a new LineCollectionNoder for the given collection of
     * linear geometries.
     *
     * @param collection input geometries; caller retains ownership.
     *   The vector must outlive this object.
     */
    LineCollectionNoder(const std::vector<const Geometry*>& collection);

    /**
     * Nodes a collection of linear geometries (vector of raw const pointers).
     *
     * @param collection vector of linear geometries
     * @param gridSize snap-rounding grid size, or 0.0 for standard noding
     * @return one noded MultiLineString per input element, in the same order
     */
    static std::vector<std::unique_ptr<Geometry>> node(
        std::vector<const Geometry*>& collection,
        double gridSize = 0.0);

    /**
     * Nodes a collection of linear geometries (vector of unique_ptr).
     *
     * @param collection vector of owning pointers to linear geometries
     * @param gridSize snap-rounding grid size, or 0.0 for standard noding
     * @return one noded MultiLineString per input element, in the same order
     */
    static std::vector<std::unique_ptr<Geometry>> node(
        const std::vector<std::unique_ptr<Geometry>>& collection,
        double gridSize = 0.0);

    /**
     * Computes the noded collection.
     *
     * @param gridSize snap-rounding grid size, or 0.0 for standard noding
     * @return one noded MultiLineString per input element, in the same order
     */
    std::vector<std::unique_ptr<Geometry>> node(double gridSize = 0.0);

    // Noncopyable
    LineCollectionNoder(const LineCollectionNoder&) = delete;
    LineCollectionNoder& operator=(const LineCollectionNoder&) = delete;

private:

    const std::vector<const Geometry*>& m_input;
    const GeometryFactory* m_geomFactory;

    /**
     * Extracts NodedSegmentStrings from the linear components of g
     * (LineString only, not LinearRing/polygon rings), tagging each
     * with reinterpret_cast<const void*>(static_cast<uintptr_t>(index)).
     */
    static void extractSegments(
        const Geometry& g,
        std::size_t index,
        std::vector<std::unique_ptr<NodedSegmentString>>& segments);

    /**
     * Builds a MultiLineString from the noded sub-segments that carry
     * the given source index tag. Deduplicates via OrientedCoordinateArray
     * (same logic as GeometryNoder::toGeometry).
     */
    std::unique_ptr<Geometry> buildResult(
        const std::vector<std::unique_ptr<SegmentString>>& nodedSegs,
        std::size_t index) const;

};

} // geos::operation::linenode
} // geos::operation
} // geos
