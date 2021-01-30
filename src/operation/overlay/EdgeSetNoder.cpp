/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/EdgeSetNoder.java rev. 1.12 (JTS-1.10)
 *
 **********************************************************************/

#include <vector>

#include <geos/operation/overlay/EdgeSetNoder.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/index/EdgeSetIntersector.h>
#include <geos/geomgraph/index/SimpleMCSweepLineIntersector.h>
#include <geos/geomgraph/index/SegmentIntersector.h>


using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geomgraph::index;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay


void
EdgeSetNoder::addEdges(std::vector<Edge*>* edges)
{
    inputEdges->insert(inputEdges->end(), edges->begin(), edges->end());
}

std::vector<Edge*>*
EdgeSetNoder::getNodedEdges()
{
    EdgeSetIntersector* esi = new SimpleMCSweepLineIntersector();
    SegmentIntersector* si = new SegmentIntersector(li, true, false);
    esi->computeIntersections(inputEdges, si, true);
    //Debug.println("has proper int = " + si.hasProperIntersection());
    std::vector<Edge*>* splitEdges = new std::vector<Edge*>();
    for(Edge* e : *inputEdges) {
        e->getEdgeIntersectionList().addSplitEdges(splitEdges);
    }
    return splitEdges;
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos
