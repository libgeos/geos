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
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.1  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 *
 **********************************************************************/


#ifndef GEOS_NODING_SNAPROUND_H
#define GEOS_NODING_SNAPROUND_H

#include <geos/platform.h>
#include <geos/noding.h>
#include <geos/geom.h>
#include <vector>

using namespace std;

namespace geos {

class SegmentSnapper {
private:
	static double TOLERANCE;
public:
	/**
	* @return true if the point p is within the snap tolerance of the line p0-p1
	*/
	static bool isWithinTolerance(const Coordinate& p,const Coordinate& p0,const Coordinate& p1);
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
	bool addSnappedNode(Coordinate& snapPt,SegmentString *segStr,int segIndex);
};

class SimpleSegmentStringsSnapper {
private:
	int nSnaps;
	/**
	* Performs a brute-force comparison of every segment in each SegmentString.
	* This has n^2 performance.
	*/
	void computeSnaps(SegmentString *e0, SegmentString *e1, SegmentSnapper *ss);
public:
	SimpleSegmentStringsSnapper();
	int getNumSnaps();
	void computeNodes(vector<SegmentString*>* edges, SegmentSnapper *ss, bool testAllSegments);
};

/**
 * Uses snap rounding to compute a rounded, noded arrangement from a
 * set of linestrings.
 *
 */
class SnapRounder {
protected:
	LineIntersector *li;
public:
	void setLineIntersector(LineIntersector *newLi);
	vector<SegmentString*>* node(vector<SegmentString*>* inputSegmentStrings);
private:	
	vector<SegmentString*>* fullyIntersectSegments(vector<SegmentString*>* segStrings, LineIntersector *aLi);
	/**
	* Computes new nodes introduced as a result of snapping segments to near vertices
	* @param li
	*/
	vector<SegmentString*>* computeSnaps(vector<SegmentString*>* segStrings);
};

}
#endif

