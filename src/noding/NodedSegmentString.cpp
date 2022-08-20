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
std::vector<Coordinate>
NodedSegmentString::getNodedCoordinates() {
    return nodeList.getSplitCoordinates();
}


/* public static */
SegmentString::NonConstVect*
NodedSegmentString::getNodedSubstrings(
    const SegmentString::NonConstVect& segStrings)
{
    SegmentString::NonConstVect* resultEdgelist = new SegmentString::NonConstVect();
    getNodedSubstrings(segStrings, resultEdgelist);
    return resultEdgelist;
}

/* virtual public */
const geom::Coordinate&
NodedSegmentString::getCoordinate(std::size_t i) const
{
    if (pts_rw) return pts_rw->getAt(i);
    else return pts_ro->getAt(i);
}

/* virtual public */
const geom::CoordinateSequence*
NodedSegmentString::getCoordinatesRO() const
{
    if (pts_rw) return pts_rw.get();
    else return pts_ro;
}

/* virtual public */
geom::CoordinateSequence*
NodedSegmentString::getCoordinatesRW()
{
    if (!pts_rw)
        pts_rw = pts_ro->clone();
    return pts_rw.get();
}

geom::CoordinateSequence*
NodedSegmentString::releaseCoordinates()
{
    if (pts_rw)
        return pts_rw.release();
    else
        return pts_ro->clone().release();
}

/* virtual public */
bool
NodedSegmentString::isClosed() const
{
    return getCoordinate(0) == getCoordinate(size() - 1);
}

/* public virtual */
std::ostream&
NodedSegmentString::print(std::ostream& os) const
{
    os << "NodedSegmentString: " << std::endl;
    if (pts_rw) {
        os << " LINESTRING" << *(pts_rw) << ";" << std::endl;
    }
    else {
        os << " LINESTRING" << *(pts_ro) << ";" << std::endl;
    }
    os << " Nodes: " << nodeList.size() << std::endl;

    return os;
}


} // geos::noding
} // geos

