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

vector<SegmentString*>* SimpleNoder::node(vector<SegmentString*>* inputSegStrings) {
	for (int i0=0;i0<(int)inputSegStrings->size();i0++) {
		SegmentString *edge0=(*inputSegStrings)[i0];
		for (int i1=0;i1<(int)inputSegStrings->size();i1++) {
			SegmentString *edge1=(*inputSegStrings)[i1];
			computeIntersects(edge0, edge1);
		}
	}
	vector<SegmentString*> *nodedSegStrings=getNodedEdges(inputSegStrings);
	return nodedSegStrings;
}

void
SimpleNoder::computeIntersects(SegmentString *e0, SegmentString *e1)
{
	const CoordinateSequence *pts0=e0->getCoordinates();
	const CoordinateSequence *pts1=e1->getCoordinates();

	unsigned int npts0=pts0->getSize();
	unsigned int npts1=pts1->getSize();
	for (unsigned int i0=0; i0<npts0-1; ++i0) {
		for (unsigned int i1=0; i1<npts1-1; ++i1) {
			segInt->processIntersections(e0, i0, e1, i1);
		}
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

