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
 * Revision 1.19  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.18  2006/01/31 19:07:33  strk
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
 * Revision 1.17  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.16  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.15  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.14  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 * Adapted to new getCoordinatesRO() interface
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <stdio.h>

namespace geos {
namespace algorithm { // geos.algorithm

MCPointInRing::MCSelecter::MCSelecter(const Coordinate& newP,MCPointInRing *prt) {
	p=newP;
	parent=prt;
}

void MCPointInRing::MCSelecter::select(LineSegment *ls) {
	parent->testLineSegment(p,ls);
}

MCPointInRing::MCPointInRing(LinearRing *newRing) {
	ring=newRing;
	tree=NULL;
	crossings=0;
	pts=NULL;
	interval=new BinTreeInterval();
    buildIndex();
}

MCPointInRing::~MCPointInRing() {
	delete tree;
	delete interval;
	delete pts;
}

void MCPointInRing::buildIndex() {
//	Envelope *env=ring->getEnvelopeInternal();
	tree=new Bintree();
	pts=CoordinateSequence::removeRepeatedPoints(ring->getCoordinatesRO());
	vector<indexMonotoneChain*> *mcList=MonotoneChainBuilder::getChains(pts);
	for(int i=0;i<(int)mcList->size();i++) {
		indexMonotoneChain *mc=(*mcList)[i];
		Envelope *mcEnv=mc->getEnvelope();
		interval->min=mcEnv->getMinY();
		interval->max=mcEnv->getMaxY();
		tree->insert(interval,mc);
	}
	delete mcList;
}

bool MCPointInRing::isInside(const Coordinate& pt) {
	crossings=0;
	// test all segments intersected by ray from pt in positive x direction
	Envelope *rayEnv=new Envelope(DoubleNegInfinity,DoubleInfinity,pt.y,pt.y);
	interval->min=pt.y;
	interval->max=pt.y;
	vector<void*> *segs=tree->query(interval);
	//System.out.println("query size=" + segs.size());
	MCSelecter *mcSelecter=new MCSelecter(pt,this);
	for(int i=0;i<(int)segs->size();i++) {
		indexMonotoneChain *mc=(indexMonotoneChain*) (*segs)[i];
		testMonotoneChain(rayEnv,mcSelecter,mc);
	}
	/*
	*  p is inside if number of crossings is odd.
	*/
//	for(int i=0;i<(int)segs->size();i++) {
//		delete (indexMonotoneChain*) (*segs)[i];
//	}
	delete segs;
	delete rayEnv;
	delete mcSelecter;
	if((crossings%2)==1) {
		return true;
	}
	return false;
}


void MCPointInRing::testMonotoneChain(Envelope *rayEnv,MCSelecter *mcSelecter,indexMonotoneChain *mc) {
	mc->select(rayEnv, mcSelecter);
}

void MCPointInRing::testLineSegment(Coordinate& p,LineSegment *seg) {
	double xInt;  // x intersection of segment with ray
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;

	/*
	*  Test if segment crosses ray from test point in positive x direction.
	*/
	Coordinate& p1=seg->p0;
	Coordinate& p2=seg->p1;
	x1=p1.x-p.x;
	y1=p1.y-p.y;
	x2=p2.x-p.x;
	y2=p2.y-p.y;
	if (((y1>0)&&(y2<=0)) || ((y2>0)&&(y1<=0))) {
		/*
		*  segment straddles x axis, so compute intersection.
		*/
		xInt=RobustDeterminant::signOfDet2x2(x1,y1,x2,y2)/(y2-y1);
		//xsave=xInt;
		/*
		*  crosses ray if strictly positive intersection.
		*/
		if (0.0<xInt) {
			crossings++;
		}
	}
}

} // namespace geos.algorithm
} // namespace geos

