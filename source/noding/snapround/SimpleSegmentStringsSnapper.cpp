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
 *
 **********************************************************************/


#include <geos/nodingSnapround.h>

namespace geos {

SimpleSegmentStringsSnapper::SimpleSegmentStringsSnapper() {
	nSnaps = 0;
}

int SimpleSegmentStringsSnapper::getNumSnaps() { 
	return nSnaps;
}


void SimpleSegmentStringsSnapper::computeNodes(vector<SegmentString*>* edges, SegmentSnapper *ss, bool testAllSegments){
	nSnaps = 0;
	for(int i0=0;i0<(int)edges->size();i0++) {
		SegmentString *edge0=(*edges)[i0];
		for(int i1=0;i1<(int)edges->size();i1++) {
			SegmentString *edge1=(*edges)[i1];
			if (testAllSegments || edge0!=edge1)
				computeSnaps(edge0,edge1,ss);
		}
	}
	cout<<"nSnaps = " << nSnaps << endl;
}

/**
* Performs a brute-force comparison of every segment in each SegmentString.
* This has n^2 performance.
*/
void SimpleSegmentStringsSnapper::computeSnaps(SegmentString *e0, SegmentString *e1, SegmentSnapper *ss){
	const CoordinateSequence *pts0=e0->getCoordinates();
	const CoordinateSequence *pts1=e1->getCoordinates();
	unsigned int npts0=pts0->getSize();
	unsigned int npts1=pts1->getSize();
	for (unsigned int i0=0; i0 <npts0-1; ++i0) {
		for (unsigned int i1=0; i1<npts1-1; ++i1) {
			bool isNodeAdded = ss->addSnappedNode((Coordinate&)pts0->getAt(i0), e1, i1);
			if (isNodeAdded) nSnaps++;
		}
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.5  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.4  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
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

