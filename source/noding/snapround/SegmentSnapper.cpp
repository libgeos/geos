/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.3  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 *
 **********************************************************************/


#include <geos/nodingSnapround.h>
#include <math.h>

namespace geos {

	double SegmentSnapper::TOLERANCE=0.5;

/**
* @return true if the point p is within the snap tolerance of the line p0-p1
*/
bool SegmentSnapper::isWithinTolerance(const Coordinate& p,const Coordinate& p0,const Coordinate& p1){
	double minx = p.x - TOLERANCE;
	double maxx = p.x + TOLERANCE;
	double miny = p.y - TOLERANCE;
	double maxy = p.y + TOLERANCE;
	double segMinx = min(p0.x, p1.x);
	double segMaxx = max(p0.x, p1.x);
	double segMiny = min(p0.y, p1.y);
	double segMaxy = max(p0.y, p1.y);
	if ( maxx < segMinx
	|| minx > segMaxx
	|| maxy < segMiny
	|| miny > segMaxy) return false;

	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;

	double px = p.x - p0.x;
	double py = p.y - p0.y;

	//double dely = px * dy / dx - py;
	//double delx = py * dx / dy - px;
	double discy = px * dy - py * dx;

	if (fabs(discy) < fabs(0.5 * dx) ) return true;
	double discx = py * dx - px * dy;
	if (fabs(discx) < fabs(0.5 * dy) ) return true;

	return false;
}

/**
* Adds a new node (equal to the snap pt) to the segment
* if the snapPt is
* within tolerance of the segment
*
* @param snapPt
* @param segStr
* @param segIndex
* @return <code>true</code> if a node was added
*/
bool SegmentSnapper::addSnappedNode(Coordinate& snapPt,SegmentString *segStr,  int segIndex){
	Coordinate p0 = segStr->getCoordinate(segIndex);
	Coordinate p1 = segStr->getCoordinate(segIndex + 1);
	// no need to snap if the snapPt equals an endpoint of the segment
	if (snapPt==p0) return false;
	if (snapPt==p1) return false;
	if (isWithinTolerance(snapPt, p0, p1)) {
		segStr->addIntersection(snapPt, segIndex);
		return true;
	}
	return false;
}
}
