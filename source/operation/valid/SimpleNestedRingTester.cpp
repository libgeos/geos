/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opValid.h>
#include <geos/util.h>
#include <stdio.h>

namespace geos {

SimpleNestedRingTester::SimpleNestedRingTester(GeometryGraph *newGraph):
	cga(new CGAlgorithms()),
	graph(newGraph),
	rings(new vector<LinearRing*>()),
	nestedPt(NULL)
{
}

SimpleNestedRingTester::~SimpleNestedRingTester(){
	delete rings;
	delete cga;
}


void
SimpleNestedRingTester::add(LinearRing *ring)
{
	rings->push_back(ring);
}

Coordinate *
SimpleNestedRingTester::getNestedPoint()
{
	return nestedPt;
}

bool
SimpleNestedRingTester::isNonNested()
{
	for(int i=0;i< (int)rings->size(); i++) {
		LinearRing* innerRing=(*rings)[i];
		CoordinateSequence *innerRingPts=innerRing->getCoordinates();
		for(int j= 0;j<(int)rings->size(); j++) {
			LinearRing* searchRing=(*rings)[j];
			CoordinateSequence *searchRingPts=searchRing->getCoordinates();
			if (innerRing==searchRing)
				continue;
			if (!innerRing->getEnvelopeInternal()->intersects(searchRing->getEnvelopeInternal()))
				continue;
			const Coordinate *innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
			Assert::isTrue(innerRingPt!=NULL, "Unable to find a ring point not a node of the search ring");

			bool isInside=cga->isPointInRing(*innerRingPt,searchRingPts);
			if (isInside) {
				/*
				 * innerRingPt is const just because the input
				 * CoordinateSequence is const. If the input
				 * Polygon survives lifetime of this object
				 * we are safe.
				 */
				nestedPt=const_cast<Coordinate *>(innerRingPt);
				return false;
			}
		}
	}
	return true;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.9  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.6  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

