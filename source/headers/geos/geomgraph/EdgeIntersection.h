/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEINTERSECTION_H
#define GEOS_GEOMGRAPH_EDGEINTERSECTION_H

#include <string>

#include <geos/geom/Coordinate.h> // for CoordinateLessThen

#include <geos/inline.h>


namespace geos {
namespace geomgraph { // geos.geomgraph

class EdgeIntersection {
public:
	geom::Coordinate coord;
	int segmentIndex;
	double dist;
	EdgeIntersection(const geom::Coordinate& newCoord, int newSegmentIndex, double newDist);
	virtual ~EdgeIntersection();
	int compare(int newSegmentIndex, double newDist) const;
	bool isEndPoint(int maxSegmentIndex);
	std::string print() const;
	int compareTo(const EdgeIntersection *) const;
};

struct EdgeIntersectionLessThen {
	bool operator()(const EdgeIntersection *ei1,
		const EdgeIntersection *ei2) const
	{
		if ( ei1->segmentIndex<ei2->segmentIndex ||
			( ei1->segmentIndex==ei2->segmentIndex &&
		     	ei1->dist<ei2->dist ) ) return true;
		return false;
	}
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/EdgeIntersection.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEINTERSECTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

