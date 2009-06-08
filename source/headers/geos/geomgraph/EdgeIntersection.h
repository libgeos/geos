/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009      Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/EdgeIntersection.java rev. 1.5 (JTS-1.10)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEINTERSECTION_H
#define GEOS_GEOMGRAPH_EDGEINTERSECTION_H

#include <geos/export.h>
#include <string>

#include <geos/geom/Coordinate.h> // for CoordinateLessThen

#include <geos/inline.h>


namespace geos {
namespace geomgraph { // geos.geomgraph

/**
 * Represents a point on an
 * edge which intersects with another edge.
 * 
 * The intersection may either be a single point, or a line segment
 * (in which case this point is the start of the line segment)
 * The intersection point must be precise.
 *
 */
class GEOS_DLL EdgeIntersection {
public:

	// the point of intersection
	geom::Coordinate coord;

	// the index of the containing line segment in the parent edge
	int segmentIndex;

	// the edge distance of this point along the containing line segment
	double dist;

	EdgeIntersection(const geom::Coordinate& newCoord,
	                 int newSegmentIndex, double newDist);

	virtual ~EdgeIntersection();

	/**
	 * @return -1 this EdgeIntersection is located before the
	 *                 argument location
	 * @return 0 this EdgeIntersection is at the argument location
	 * @return 1 this EdgeIntersection is located after the argument
	 *                location
	 */
	int compare(int newSegmentIndex, double newDist) const;

	bool isEndPoint(int maxSegmentIndex);

	std::string print() const;

	int compareTo(const EdgeIntersection *) const;

	const geom::Coordinate& getCoordinate() const {
		return coord;
	}

	int getSegmentIndex() const { return segmentIndex; }

	double getDistance() { return dist; }

};

struct GEOS_DLL  EdgeIntersectionLessThen {
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

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeIntersection.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEINTERSECTION_H


