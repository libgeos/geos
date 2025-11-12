/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/BoundaryChainNoder.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentString.h>


using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;


namespace geos {   // geos
namespace noding { // geos::noding

/* public */
void
BoundaryChainNoder::computeNodes(const std::vector<SegmentString*>& segStrings)
{
    SegmentSet boundarySegSet;
    std::vector<BoundaryChainMap> boundaryChains;
    boundaryChains.reserve(segStrings.size());
    addSegments(segStrings, boundarySegSet, boundaryChains);
    markBoundarySegments(boundarySegSet);
    m_chainList = extractChains(boundaryChains);

    Coordinate::UnorderedSet nodePts = findNodePts(m_chainList);
    if (!nodePts.empty()) {
        // At this point we have copied all the SegmentString*
        // we want to keep, so the container needs to go away and be replaced
        m_chainList = nodeChains(m_chainList, nodePts);
    }
}

/* private */
Coordinate::UnorderedSet
BoundaryChainNoder::findNodePts(const std::vector<std::unique_ptr<SegmentString>>& segStrings) const
{
    Coordinate::UnorderedSet interiorVertices;
    Coordinate::UnorderedSet nodes;
    for (const auto& ss : segStrings) {
        //-- endpoints are nodes
        nodes.insert(ss->getCoordinate(0));
        nodes.insert(ss->getCoordinate(ss->size() - 1));

        //-- check for duplicate interior points
        for (std::size_t i = 1; i < ss->size() - 1; i++) {
            const Coordinate& p = ss->getCoordinate(i);
            if (interiorVertices.find(p) != interiorVertices.end()) {
                nodes.insert(p);
            }
            interiorVertices.insert(p);
        }
    }
    return nodes;
}

/* private */
std::vector<std::unique_ptr<SegmentString>>
BoundaryChainNoder::nodeChains(
    std::vector<std::unique_ptr<SegmentString>>& chains,
    const Coordinate::UnorderedSet& nodePts)
{
    std::vector<std::unique_ptr<SegmentString>> nodedChains;
    for (auto& chain : chains) {
        nodeChain(std::move(chain), nodePts, nodedChains);
    }
    return nodedChains;
}


/* private */
void
BoundaryChainNoder::nodeChain(
    std::unique_ptr<SegmentString> chain,
    const Coordinate::UnorderedSet& nodePts,
    std::vector<std::unique_ptr<SegmentString>>& nodedChains)
{
    std::size_t start = 0;
    while (start < chain->size() - 1) {
        std::size_t end = findNodeIndex(chain.get(), start, nodePts);
        //-- if no interior nodes found, keep original chain
        if (start == 0 && end == chain->size() - 1) {
            nodedChains.emplace_back(std::move(chain));
            return;
        }
        nodedChains.push_back(substring(chain.get(), start, end));
        start = end;
    }
}

/* private static */
std::unique_ptr<BasicSegmentString>
BoundaryChainNoder::substring(const SegmentString* segString, std::size_t start, std::size_t end)
{
    // FIXME: Doesn't this leak "pts" ?
    auto pts = std::make_unique<CoordinateSequence>();
    pts->add(*segString->getCoordinates(), start, end);
    return std::make_unique<BasicSegmentString>(pts.release(), segString->getData());
}


/* private */
std::size_t
BoundaryChainNoder::findNodeIndex(
    const SegmentString* chain,
    std::size_t start,
    const Coordinate::UnorderedSet& nodePts) const
{
    for (std::size_t i = start + 1; i < chain->size(); i++) {
        if (nodePts.find(chain->getCoordinate(i)) != nodePts.end())
            return i;
    }
    return chain->size() - 1;
}


/* public */
std::vector<std::unique_ptr<SegmentString>>
BoundaryChainNoder::getNodedSubstrings()
{
    return std::move(m_chainList);
}

/* private */
void
BoundaryChainNoder::addSegments(
    const std::vector<SegmentString*>& segStrings,
    SegmentSet& segSet,
    std::vector<BoundaryChainMap>& boundaryChains)
{
    for (SegmentString* ss : segStrings) {
        m_constructZ |= ss->getCoordinates()->hasZ();
        m_constructM |= ss->getCoordinates()->hasM();

        boundaryChains.emplace_back(ss);
        BoundaryChainMap& chainMap = boundaryChains.back();
        addSegments(ss, chainMap, segSet);
    }
}

/* private static */
bool
BoundaryChainNoder::segSetContains(SegmentSet& segSet, Segment& seg)
{
    auto search = segSet.find(seg);
    if(search != segSet.end()) {
        return true;
    }
    else {
        return false;
    }
}

/* private static */
void
BoundaryChainNoder::addSegments(
    SegmentString* segString,
    BoundaryChainMap& chainMap,
    SegmentSet& segSet)
{
    const CoordinateSequence& segCoords = *segString->getCoordinates();

    for (std::size_t i = 0; i < segString->size() - 1; i++) {
        Segment seg(segCoords, chainMap, i);
        if (segSetContains(segSet, seg)) {
            segSet.erase(seg);
        }
        else {
            segSet.insert(seg);
        }
    }
}


/* private static */
void
BoundaryChainNoder::markBoundarySegments(SegmentSet& segSet)
{
    for (const Segment& seg : segSet) {
        seg.markBoundary();
    }
}

/* private */
std::vector<std::unique_ptr<SegmentString>>
BoundaryChainNoder::extractChains(std::vector<BoundaryChainMap>& boundaryChains) const
{
    std::vector<std::unique_ptr<SegmentString>> chains;
    for (BoundaryChainMap& chainMap : boundaryChains) {
        chainMap.createChains(chains, m_constructZ, m_constructM);
    }
    return chains;
}

/*************************************************************************
 * BoundarySegmentMap
 */

/* public */
void
BoundaryChainNoder::BoundaryChainMap::setBoundarySegment(std::size_t index)
{
    isBoundary[index] = true;
}

/* public */
void
BoundaryChainNoder::BoundaryChainMap::createChains(
    std::vector<std::unique_ptr<SegmentString>>& chains,
    bool constructZ,
    bool constructM)
{
    std::size_t endIndex = 0;
    while (true) {
        std::size_t startIndex = findChainStart(endIndex);
        if (startIndex >= segString->size() - 1)
            break;
        endIndex = findChainEnd(startIndex);
        auto ss = createChain(segString, startIndex, endIndex, constructZ, constructM);
        chains.push_back(std::move(ss));
    }
}


/* private static */
std::unique_ptr<SegmentString>
BoundaryChainNoder::BoundaryChainMap::createChain(
    const SegmentString *segString,
    std::size_t startIndex,
    std::size_t endIndex,
    bool constructZ,
    bool constructM)
{
    auto npts = endIndex - startIndex + 1;
    auto pts = detail::make_unique<CoordinateSequence>(0, constructZ, constructM);
    pts->reserve(npts);
    pts->add(*segString->getCoordinates(), startIndex, endIndex);

    return std::make_unique<NodedSegmentString>(pts.release(), constructZ, constructM, segString->getData());
}

/* private */
std::size_t
BoundaryChainNoder::BoundaryChainMap::findChainStart(std::size_t index) const
{
    while (index < isBoundary.size() && ! isBoundary[index]) {
        index++;
    }
    return index;
}

/* private */
std::size_t
BoundaryChainNoder::BoundaryChainMap::findChainEnd(std::size_t index) const
{
    index++;
    while (index < isBoundary.size() && isBoundary[index]) {
        index++;
    }
    return index;
}


} // geos::noding
} // geos
