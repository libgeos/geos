/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/distance/EuclideanDistanceToPoint.java 1.1 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_DISTANCE_EUCLIDEANDISTANCETOPOINT_H
#define GEOS_ALGORITHM_DISTANCE_EUCLIDEANDISTANCETOPOINT_H

#include <geos/geom/LineSegment.h> // for composition

namespace geos {
	namespace algorithm {
		namespace distance {
			class PointPairDistance;
		}
	}
	namespace geom {
		class Geometry;
		class Coordinate; 
		class LineString; 
		class Polygon; 
	}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace distance { // geos::algorithm::distance

/**
 * Computes the Euclidean distance (L2 metric) from a Point to a Geometry.
 *
 * Also computes two points which are separated by the distance.
 */
class EuclideanDistanceToPoint
{
public:

	EuclideanDistanceToPoint() {}

	static void computeDistance(const geom::Geometry& geom,
			            const geom::Coordinate& pt,
	                            PointPairDistance& ptDist);

	static void computeDistance(const geom::LineString& geom,
			            const geom::Coordinate& pt,
	                            PointPairDistance& ptDist);

	static void computeDistance(const geom::LineSegment& geom,
			            const geom::Coordinate& pt,
	                            PointPairDistance& ptDist);

	static void computeDistance(const geom::Polygon& geom,
			            const geom::Coordinate& pt,
	                            PointPairDistance& ptDist);

private:

	// used for point-line distance calculation
	// NOT THREAD SAFE !!
	static geom::LineSegment tempSegment;
};

} // geos::algorithm::distance
} // geos::algorithm
} // geos

#endif // GEOS_ALGORITHM_DISTANCE_EUCLIDEANDISTANCETOPOINT_H

/**********************************************************************
 * $Log$
 **********************************************************************/

