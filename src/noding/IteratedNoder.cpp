/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/IteratedNoder.java r591 (JTS-1.12+)
 *
 **********************************************************************/

#include <sstream>
#include <vector>

#include <geos/profiler.h>
#include <geos/util/TopologyException.h>
#include <geos/noding/IteratedNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/IntersectionAdder.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/* private */
void
IteratedNoder::node(const std::vector<SegmentString*>& segStrings,
                    int& numInteriorIntersections,
                    CoordinateXY& intersectionPoint)
{
    IntersectionAdder si(li);
    MCIndexNoder noder;
    noder.setSegmentIntersector(&si);
    noder.computeNodes(segStrings);
    auto updatedSegStrings = noder.getNodedSubstrings();
    nodedSegStrings = std::move(updatedSegStrings);
    numInteriorIntersections = si.numInteriorIntersections;

    if (si.hasProperInteriorIntersection()) {
        intersectionPoint = si.getProperIntersectionPoint();
    }
}

/* public */
void
IteratedNoder::computeNodes(const std::vector<SegmentString*>& segStrings)
{
    int numInteriorIntersections;
    int nodingIterationCount = 0;
    int lastNodesCreated = -1;
    CoordinateXY intersectionPoint = CoordinateXY::getNull();

    bool firstPass = true;
    do  {
        // NOTE: will change this.nodedSegStrings
        if (firstPass) {
            node(segStrings, numInteriorIntersections, intersectionPoint);
            firstPass = false;
        } else {
            auto nodingInput = SegmentString::toRawPointerVector(nodedSegStrings);
            node(nodingInput, numInteriorIntersections, intersectionPoint);
        }

        nodingIterationCount++;
        int nodesCreated = numInteriorIntersections;

        /*
         * Fail if the number of nodes created is not declining.
         * However, allow a few iterations at least before doing this
         */
        if(lastNodesCreated > 0
                && nodesCreated >= lastNodesCreated
                && nodingIterationCount > maxIter) {

            std::stringstream s;
            s << "Iterated noding failed to converge after " <<
              nodingIterationCount << " iterations (near " <<
              intersectionPoint << ")";
            throw util::TopologyException(s.str());
        }
        lastNodesCreated = nodesCreated;

    }
    while(lastNodesCreated > 0);
    //cerr<<"# nodings = "<<nodingIterationCount<<endl;
}


} // namespace geos.noding
} // namespace geos

