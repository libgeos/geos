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
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/graph.h"
#include <stdio.h>

namespace geos {

EdgeIntersection::EdgeIntersection(const Coordinate& newCoord, int newSegmentIndex, double newDist) {
	coord=newCoord;
	segmentIndex=newSegmentIndex;
	dist=newDist;
}

EdgeIntersection::~EdgeIntersection() {
	//aaa
}

int EdgeIntersection::compare(int newSegmentIndex, double newDist) {
	if (segmentIndex<newSegmentIndex) return -1;
	if (segmentIndex>newSegmentIndex) return 1;
	if (dist<newDist) return -1;
	if (dist>newDist) return 1;
	return 0;
}

bool EdgeIntersection::isEndPoint(int maxSegmentIndex) {
	if (segmentIndex==0 && dist==0.0) return true;
	if (segmentIndex==maxSegmentIndex) return true;
	return false;
}

string EdgeIntersection::print(){
	string out="";
	out+=coord.toString();
	char buffer[255];
	sprintf(buffer," seg # = %i dist= %g",segmentIndex,dist);
	string out2(buffer);
	out.append(out2);
	return out;
}
}

