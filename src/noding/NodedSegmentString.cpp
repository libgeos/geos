/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/NodedSegmentString.java r320 (JTS-1.12)
 *
 **********************************************************************/


#include <geos/noding/NodedSegmentString.h>
#include <geos/algorithm/LineIntersector.h>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos::noding

const SegmentNodeList&
NodedSegmentString::getNodeList() const
{
    return nodeList;
}

SegmentNodeList&
NodedSegmentString::getNodeList()
{
    return nodeList;
}

/* public static */
void
NodedSegmentString::getNodedSubstrings(
    const SegmentString::NonConstVect& segStrings,
    SegmentString::NonConstVect* resultEdgeList)
{
    assert(resultEdgeList);
    for(SegmentString::NonConstVect::const_iterator
            i = segStrings.begin(), iEnd = segStrings.end();
            i != iEnd; ++i) {
        NodedSegmentString* ss = dynamic_cast<NodedSegmentString*>(*i);
        assert(ss);
        ss->getNodeList().addSplitEdges(resultEdgeList);
    }
}

/* public */
std::unique_ptr<CoordinateSequence>
NodedSegmentString::getNodedCoordinates() {
    return nodeList.getSplitCoordinates();
}


/* public static */
SegmentString::NonConstVect*
NodedSegmentString::getNodedSubstrings(
    const SegmentString::NonConstVect& segStrings)
{
    SegmentString::NonConstVect* resultEdgelist = \
            new SegmentString::NonConstVect();
    getNodedSubstrings(segStrings, resultEdgelist);
    return resultEdgelist;
}

std::unique_ptr<geom::CoordinateSequence>
NodedSegmentString::releaseCoordinates()
{
    auto ret = std::unique_ptr<CoordinateSequence>(seq);
    seq = nullptr;
    return ret;
}

/* public virtual */
std::ostream&
NodedSegmentString::print(std::ostream& os) const
{
    os << "NodedSegmentString: " << std::endl;
    os << " LINESTRING" << *(getCoordinates()) << ";" << std::endl;
    os << " Nodes: " << nodeList.size() << std::endl;

    return os;
}


} // geos::noding
} // geos

