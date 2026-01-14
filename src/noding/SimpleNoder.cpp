/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SimpleNoder.java rev. 1.7 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/SimpleNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/ArcIntersector.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/NodableArcString.h>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

template<typename T1, typename T2>
static void computeIntersectsImpl(ArcIntersector& intersector, T1& e0, T2& e1)
{
    for(std::size_t i = 0; i < e0.getSize(); i++) {
        for(std::size_t j = 0; j < e1.getSize(); j++) {
            intersector.processIntersections(e0, i, e1, j);
        }
    }
}

/*private*/
void
SimpleNoder::computeIntersects(PathString& e0, PathString& e1)
{
    assert(m_intersector); // must provide a segment intersector!

    SegmentString* linear0 = dynamic_cast<SegmentString*>(&e0);
    SegmentString* linear1 = dynamic_cast<SegmentString*>(&e1);

    if (linear0 && linear1) {
        computeIntersectsImpl(*m_intersector, *linear0, *linear1);
    } else if (!linear0 && !linear1) {
        computeIntersectsImpl(*m_intersector, *detail::down_cast<ArcString*>(&e0), *detail::down_cast<ArcString*>(&e1));
    } else if (!linear0) {
        computeIntersectsImpl(*m_intersector, *detail::down_cast<ArcString*>(&e0), *linear1);
    } else {
        computeIntersectsImpl(*m_intersector, *linear0, *detail::down_cast<ArcString*>(&e1));
    }
}

/*public*/
void
SimpleNoder::computePathNodes(const std::vector<PathString*>& inputPathStrings)
{
    m_pathStrings = inputPathStrings;

    for (auto* edge0: m_pathStrings) {
        for (auto* edge1: m_pathStrings) {
            computeIntersects(*edge0, *edge1);
        }
    }
}

std::vector<std::unique_ptr<PathString>>
SimpleNoder::getNodedPaths()
{
    std::vector<std::unique_ptr<PathString>> nodedPaths;

    for (PathString* ps : m_pathStrings) {
        if (auto* nss = dynamic_cast<NodedSegmentString*>(ps)) {
            std::vector<std::unique_ptr<SegmentString>> tmp;
            nss->getNodeList().addSplitEdges(tmp);
            for (auto& segString : tmp) {
                nodedPaths.push_back(std::move(segString));
            }
        } else {
            auto* nas = detail::down_cast<NodableArcString*>(ps);
            nodedPaths.push_back(nas->getNoded());
        }
    }

    return nodedPaths;
}

} // namespace geos.noding
} // namespace geos
