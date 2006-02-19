/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/04/13 12:29:21  strk
 * GeometryLocation const-correctness.
 *
 * Revision 1.2  2004/04/13 10:05:51  strk
 * GeometryLocation constructor made const-correct.
 * Fixed erroneus down-casting in DistanceOp::computeMinDistancePoints.
 *
 * Revision 1.1  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opDistance.h>

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/**
* Constructs a GeometryLocation specifying a point on a geometry, as well as the 
* segment that the point is on (or INSIDE_AREA if the point is not on a segment).
*/
GeometryLocation::GeometryLocation(const Geometry *newComponent, int newSegIndex, const Coordinate &newPt){
	component = newComponent;
	segIndex = newSegIndex;
	pt = newPt;
}

/**
* Constructs a GeometryLocation specifying a point inside an area geometry.
*/  
GeometryLocation::GeometryLocation(const Geometry *newComponent, const Coordinate &newPt){
	component = newComponent;
	segIndex = INSIDE_AREA;
	pt = newPt;
}

/**
* Returns the geometry associated with this location.
*/
const Geometry* GeometryLocation::getGeometryComponent() { 
	return component;
}
/**
* Returns the segment index for this location. If the location is inside an
* area, the index will have the value INSIDE_AREA;
*
* @return the segment index for the location, or INSIDE_AREA
*/
int GeometryLocation::getSegmentIndex() { 
	return segIndex;
}
/**
* Returns the location.
*/
Coordinate& GeometryLocation::getCoordinate() { 
	return pt;
}

bool GeometryLocation::isInsideArea() { 
	return segIndex == INSIDE_AREA;
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos
