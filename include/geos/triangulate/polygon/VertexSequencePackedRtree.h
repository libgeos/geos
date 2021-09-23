/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Envelope;
}
}

using geos::geom::Coordinate;
using geos::geom::Envelope;


namespace geos {
namespace triangulate {
namespace polygon {


/**
 * A semi-static spatial index for points which occur
 * in a spatially-coherent sequence.
 * In particular, this is suitable for indexing the vertices
 * of a geos::geom::LineString or geos::geom::Polygon ring.
 *
 * The index is constructed in a batch fashion on a given sequence of coordinates.
 * Coordinates can be removed via the remove() method.
 *
 * Note that this index queries only the individual points
 * of the input coordinate sequence,
 * **not** any line segments which might be lie between them.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL VertexSequencePackedRtree {

private:


    /**
    * Number of items/nodes in a parent node.
    * Determined empirically.  Performance is not too sensitive to this.
    */
    static constexpr std::size_t NODE_CAPACITY = 16;

    // Members
    const std::vector<Coordinate>& items;
    std::vector<bool> removedItems;
    std::vector<std::size_t> levelOffset;
    std::size_t nodeCapacity = NODE_CAPACITY;
    std::vector<Envelope> bounds;


    // Methods

    void build();

    /**
    * Computes the level offsets.
    * This is the position in the <tt>bounds</tt> array of each level.
    *
    * The levelOffsets array includes a sentinel value of offset[0] = 0.
    * The top level is always of size 1,
    * and so also indicates the total number of bounds.
    *
    * @return the level offsets
    */
    std::vector<std::size_t> computeLevelOffsets();

    static std::size_t ceilDivisor(std::size_t num, std::size_t denom);
    static std::size_t clampMax(std::size_t x, std::size_t max);

    std::size_t levelNodeCount(std::size_t numNodes);

    std::vector<Envelope> createBounds();

    void fillItemBounds(std::vector<Envelope>& bounds);
    void fillLevelBounds(std::size_t lvl, std::vector<Envelope>& bounds);

    static Envelope computeNodeEnvelope(const std::vector<Envelope>& bounds,
        std::size_t start, std::size_t end);
    static Envelope computeItemEnvelope(const std::vector<Coordinate>& items,
        std::size_t start, std::size_t end);

    void queryNode(const Envelope& queryEnv,
        std::size_t level, std::size_t nodeIndex,
        std::vector<std::size_t>& result) const;
    void queryNodeRange(const Envelope& queryEnv,
        std::size_t level, std::size_t nodeStartIndex,
        std::vector<std::size_t>& result) const;
    void queryItemRange(const Envelope& queryEnv, std::size_t itemIndex,
        std::vector<std::size_t>& result) const;

    std::size_t levelSize(std::size_t level) const;
    bool isNodeEmpty(std::size_t level, std::size_t index);
    bool isItemsNodeEmpty(std::size_t nodeIndex);


public:

    /**
    * Creates a new tree over the given sequence of coordinates.
    * The sequence should be spatially coherent to provide query performance.
    *
    * @param pts a sequence of points
    */
    VertexSequencePackedRtree(const std::vector<Coordinate>& pts);

    std::vector<Envelope> getBounds();

    /**
    * Removes the input item at the given index from the spatial index.
    *
    * @param index the index of the item in the input
    */
    void remove(std::size_t index);

    /**
    * Queries the index to find all items which intersect an extent.
    * The query result is a list of the indices of input coordinates
    * which intersect the extent.
    *
    * @param queryEnv the query extent
    * @param result vector to fill with results
    * @return
    */
    void query(const Envelope& queryEnv, std::vector<std::size_t>& result) const;

};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

