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
 **********************************************************************/

#include <geos/noding.h>

namespace geos {

vector<SegmentString*>*
Noder::getNodedEdges(vector<SegmentString*>* segStrings)
{
	vector<SegmentString*>* resultEdgelist=new vector<SegmentString*>();
	getNodedEdges(segStrings, resultEdgelist);
	return resultEdgelist;
}

void
Noder::getNodedEdges(vector<SegmentString*>* segStrings, vector<SegmentString*>* resultEdgelist)
{
	for (int i=0; i<(int)segStrings->size();i++) {
		SegmentString *ss=(*segStrings)[i];
		ss->getIntersectionList()->addSplitEdges(resultEdgelist);
	}
}

void
Noder::setSegmentIntersector(nodingSegmentIntersector *newSegInt)
{
	segInt=newSegInt;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.5  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.4  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.2  2004/05/03 12:09:22  strk
 * newline added at end of file
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

