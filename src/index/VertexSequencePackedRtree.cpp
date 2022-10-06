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



#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/index/VertexSequencePackedRtree.h>

#include <algorithm>

namespace geos {
namespace index {


/**
* Creates a new tree over the given sequence of coordinates.
* The sequence should be spatially coherent to provide query performance.
*
* @param pts a sequence of points
*/
/* public */
VertexSequencePackedRtree::VertexSequencePackedRtree(const CoordinateSequence& pts)
    : items(pts)
    , removedItems(pts.size(), false)
{
    build();
}

/* public */
std::vector<Envelope>
VertexSequencePackedRtree::getBounds()
{
    std::vector<Envelope> boundsCopy = bounds;
    // std::vector<Envelope> boundsCopy;
    // std::copy(bounds.begin(), bounds.end(), boundsCopy.begin());
    return boundsCopy;
}

//-- Index Build --------------------------------------------------------------

/* private */
void
VertexSequencePackedRtree::build()
{
    levelOffset = computeLevelOffsets();
    bounds = createBounds();
}

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
/* private */
std::vector<std::size_t>
VertexSequencePackedRtree::computeLevelOffsets()
{
    std::vector<std::size_t> offsets;
    offsets.push_back(0u);
    std::size_t szLevel = items.size();
    std::size_t currOffset = 0;
    do {
        szLevel = levelNodeCount(szLevel);
        currOffset += szLevel;
        offsets.push_back(currOffset);
    } while (szLevel > 1);
    return offsets;
}

/* private static */
std::size_t
VertexSequencePackedRtree::ceilDivisor(std::size_t num, std::size_t denom)
{
    std::size_t div = num / denom;
    return div * denom >= num ? div : div + 1;
}

/* private */
std::size_t
VertexSequencePackedRtree::levelNodeCount(std::size_t numNodes)
{
    return ceilDivisor(numNodes, nodeCapacity);
}

/* private */
std::vector<Envelope>
VertexSequencePackedRtree::createBounds()
{
    std::size_t bndsSize = levelOffset[levelOffset.size() - 1] + 1;
    std::vector<Envelope> bnds(bndsSize);
    fillItemBounds(bnds);

    for (std::size_t lvl = 1; lvl < levelOffset.size(); lvl++) {
        fillLevelBounds(lvl, bnds);
    }
    return bnds;
}


/* private */
void
VertexSequencePackedRtree::fillItemBounds(std::vector<Envelope>& bnds)
{
    std::size_t nodeStart = 0;
    std::size_t bndIndex = 0;
    do {
        std::size_t nodeEnd = clampMax(nodeStart + nodeCapacity, items.size());
        bnds[bndIndex++] = computeItemEnvelope(items, nodeStart, nodeEnd);
        nodeStart = nodeEnd;
    }
    while (nodeStart < items.size());
}


/* private */
void
VertexSequencePackedRtree::fillLevelBounds(std::size_t lvl, std::vector<Envelope>& bnds)
{
    std::size_t levelStart = levelOffset[lvl - 1];
    std::size_t levelEnd = levelOffset[lvl];
    std::size_t nodeStart = levelStart;
    std::size_t levelBndIndex = levelOffset[lvl];
    do {
        std::size_t nodeEnd = clampMax(nodeStart + nodeCapacity, levelEnd);
        bnds[levelBndIndex++] = computeNodeEnvelope(bnds, nodeStart, nodeEnd);
        nodeStart = nodeEnd;
    }
    while (nodeStart < levelEnd);
}


/* private static */
Envelope
VertexSequencePackedRtree::computeNodeEnvelope(const std::vector<Envelope>& bnds,
    std::size_t start, std::size_t end)
{
    Envelope env;
    for (std::size_t i = start; i < end; i++) {
        env.expandToInclude(bnds[i]);
    }
    return env;
}

/* private static */
Envelope
VertexSequencePackedRtree::computeItemEnvelope(const geom::CoordinateSequence& items,
    std::size_t start, std::size_t end)
{
    Envelope env;
    for (std::size_t i = start; i < end; i++) {
        env.expandToInclude(items[i]);
    }
    return env;
}

//-- Index Query --------------------------------------------------------------

/**
* Queries the index to find all items which intersect an extent.
* The query result is a list of the indices of input coordinates
* which intersect the extent.
*
* @param queryEnv the query extent
* @return an array of the indices of the input coordinates
*/
/* public */
void
VertexSequencePackedRtree::query(const Envelope& queryEnv, std::vector<std::size_t>& result) const
{
    std::size_t level = levelOffset.size() - 1;
    queryNode(queryEnv, level, 0, result);
}


/* private */
void
VertexSequencePackedRtree::queryNode(const Envelope& queryEnv,
    std::size_t level, std::size_t nodeIndex,
    std::vector<std::size_t>& result) const
{
    std::size_t boundsIndex = levelOffset[level] + nodeIndex;
    Envelope nodeEnv = bounds[boundsIndex];

    //--- node is empty
    if (nodeEnv.isNull())
        return;
    if (! queryEnv.intersects(nodeEnv))
        return;

    std::size_t childNodeIndex = nodeIndex * nodeCapacity;
    if (level == 0) {
        queryItemRange(queryEnv, childNodeIndex, result);
    }
    else {
        queryNodeRange(queryEnv, level - 1, childNodeIndex, result);
    }
}

/* private */
void
VertexSequencePackedRtree::queryNodeRange(const Envelope& queryEnv,
    std::size_t level, std::size_t nodeStartIndex,
    std::vector<std::size_t>& result) const
{
    std::size_t levelMax = levelSize(level);
    for (std::size_t i = 0; i < nodeCapacity; i++) {
        std::size_t index = nodeStartIndex + i;
        if (index >= levelMax)
            return;
        queryNode(queryEnv, level, index, result);
    }
}


/* private */
std::size_t
VertexSequencePackedRtree::levelSize(std::size_t level) const
{
    return levelOffset[level + 1] - levelOffset[level];
}


/* private */
void
VertexSequencePackedRtree::queryItemRange(const Envelope& queryEnv,
    std::size_t itemIndex, std::vector<std::size_t>& result) const
{
    for (std::size_t i = 0; i < nodeCapacity; i++) {
        std::size_t index = itemIndex + i;
        if (index >= items.size())
            return;
        const Coordinate& p = items[index];
        bool removed = removedItems[index];
        if ( (!removed) && queryEnv.contains(p))
            result.push_back(index);
    }
}

//-- Index Modify --------------------------------------------------------------

/**
* Removes the input item at the given index from the spatial index.
*
* @param index the index of the item in the input
*/
/* public */
void
VertexSequencePackedRtree::remove(std::size_t index)
{
    removedItems[index] = true;

    //--- prune the item parent node if all its items are removed
    std::size_t nodeIndex = index / nodeCapacity;
    if (! isItemsNodeEmpty(nodeIndex))
        return;

    bounds[nodeIndex].setToNull();

    if (levelOffset.size() <= 2)
        return;

    //-- prune the node parent if all children removed
    std::size_t nodeLevelIndex = nodeIndex / nodeCapacity;
    if (! isNodeEmpty(1, nodeLevelIndex))
        return;

    std::size_t nodeIndex1 = levelOffset[1] + nodeLevelIndex;
    bounds[nodeIndex1].setToNull();

    // TODO: propagate removal up the tree nodes?
}

/* private */
bool
VertexSequencePackedRtree::isNodeEmpty(std::size_t level, std::size_t index)
{
    std::size_t start = index * nodeCapacity;
    std::size_t end = clampMax(start + nodeCapacity, levelOffset[level]);
    for (std::size_t i = start; i < end; i++) {
        if (!bounds[i].isNull())
            return false;
    }
    return true;
}

/* private */
bool
VertexSequencePackedRtree::isItemsNodeEmpty(std::size_t nodeIndex)
{
    std::size_t start = nodeIndex * nodeCapacity;
    std::size_t end = clampMax(start + nodeCapacity, items.size());
    for (std::size_t i = start; i < end; i++) {
        if (!removedItems[i]) return false;
    }
    return true;
}

/* private static */
std::size_t
VertexSequencePackedRtree::clampMax(std::size_t x, std::size_t max)
{
    return x > max ? max: x;
}


} // namespace geos.index
} // namespace geos

