/**********************************************************************
 * $Id
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#ifndef GEOS_NODING_NODEDSEGMENTSTRING_H
#define GEOS_NODING_NODEDSEGMENTSTRING_H

#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/NodableSegmentString.h>
#include <geos/noding/SegmentNode.h>
#include <geos/noding/SegmentNodeList.h>
#include <geos/noding/SegmentString.h>
//#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>

//using namespace 

namespace geos {
namespace noding { // geos::noding

class NodedSegmentString : public NodableSegmentString 
{
private:
protected:
public:
	static void getNodedSubstrings( SegmentString::ConstVect * segStrings, SegmentString::NonConstVect * resultEdgelist)
	{
		for (size_t i=0, n=segStrings->size(); i<n; i++)
		{
			NodedSegmentString * nss = (NodedSegmentString *)((*segStrings)[i]);
			nss->getNodeList().addSplitEdges( resultEdgelist);
		}
	}

	/**
	 * Creates a new segment string from a list of vertices.
	 *
	 * @param pts the vertices of the segment string
	 * @param data the user-defined data of this segment string (may be null)
	 */
	NodedSegmentString( geom::CoordinateSequence *newPts, const void* newContext)
		: NodableSegmentString( newPts, newContext ) 
	{ }

	~NodedSegmentString()
	{ }

	/**
	 * Adds an intersection node for a given point and segment to this segment string.
	 * If an intersection already exists for this exact location, the existing
	 * node will be returned.
	 * 
	 * @param intPt the location of the intersection
	 * @param segmentIndex the index of the segment containing the intersection
	 * @return the intersection node for the point
	 */
	SegmentNode * addIntersectionNode( geom::Coordinate * intPt, int segmentIndex) 
	{
		int normalizedSegmentIndex = segmentIndex;

		// normalize the intersection point location
		int nextSegIndex = normalizedSegmentIndex + 1;
		if (nextSegIndex < size()) 
		{
			const geom::Coordinate &nextPt = getCoordinate( nextSegIndex);

			// Normalize segment index if intPt falls on vertex
			// The check for point equality is 2D only - Z values are ignored
			if ( intPt->equals2D( nextPt )) 
			{
				normalizedSegmentIndex = nextSegIndex;
			}
		}

		// Add the intersection point to edge intersection list.
		SegmentNode * ei = getNodeList().add( *intPt, normalizedSegmentIndex);
		return ei;
	}

};

} // namespace geos::noding
} // namespace geos

#endif // GEOS_NODING_NODEDSEGMENTSTRING_H
/**********************************************************************
 * $Log$
 **********************************************************************/

