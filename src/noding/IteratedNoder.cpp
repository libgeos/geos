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

#include <functional>
#include <sstream>
#include <vector>

#include <geos/profiler.h>
#include <geos/util/TopologyException.h>
#include <geos/noding/ArcIntersectionAdder.h>
#include <geos/noding/ArcNoder.h>
#include <geos/noding/IteratedNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/IntersectionAdder.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

namespace geos {
namespace noding { // geos.noding

IteratedNoder::IteratedNoder(const geom::PrecisionModel* newPm,
                             std::function<std::unique_ptr<Noder>()> noderFunction)
    :
    pm(newPm),
    cai(pm),
    li(pm),
    maxIter(MAX_ITER),
    m_noderFunction(noderFunction)
{
}

std::unique_ptr<Noder>
IteratedNoder::createDefaultNoder()
{
    return std::make_unique<MCIndexNoder>();
}

IteratedNoder::~IteratedNoder() = default;

/* private */
void
IteratedNoder::node(const std::vector<PathString*>& pathStrings,
                    int& numInteriorIntersections,
                    geom::CoordinateXY& intersectionPoint)
{

    auto noder = m_noderFunction();
    if (auto* spn = dynamic_cast<SinglePassNoder*>(noder.get())) {
        IntersectionAdder si(li);
        spn->setSegmentIntersector(&si);
        // TODO need to have previously checked that all inputs are SegmentStrings

        std::vector<SegmentString*> segStrings(pathStrings.size());
        for (size_t i = 0; i < pathStrings.size(); i++) {
            segStrings[i] = detail::down_cast<SegmentString*>(pathStrings[i]);
        }

        noder->computeNodes(segStrings);

        auto nodedSegStrings = noder->getNodedSubstrings();
        nodedPaths.resize(nodedSegStrings.size());
        for (size_t i = 0; i < nodedSegStrings.size(); i++) {
            nodedPaths[i].reset(nodedSegStrings[i].release());
        }

        numInteriorIntersections = si.numInteriorIntersections;

        if (si.hasProperInteriorIntersection()) {
            intersectionPoint = si.getProperIntersectionPoint();
        }
    } else {
        auto* arcNoder = detail::down_cast<ArcNoder*>(noder.get());
        auto aia = std::make_unique<ArcIntersectionAdder>(cai);
        arcNoder->setArcIntersector(std::move(aia));
        arcNoder->computePathNodes(pathStrings);
        nodedPaths = arcNoder->getNodedPaths();

        // FIXME use actual number!
        numInteriorIntersections = 0;


        // numInteriorIntersections?
        // intesectionPoint?
    }
}

/* public */
void
IteratedNoder::computePathNodes(const std::vector<PathString*>& paths)
{
    int numInteriorIntersections;
    int nodingIterationCount = 0;
    int lastNodesCreated = -1;
    geom::CoordinateXY intersectionPoint = geom::CoordinateXY::getNull();

    bool firstPass = true;
    do  {
        // NOTE: will change this.nodedSegStrings
        if (firstPass) {
            node(paths, numInteriorIntersections, intersectionPoint);
            firstPass = false;
        } else {
            auto nodingInput = PathString::toRawPointerVector(nodedPaths);
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

