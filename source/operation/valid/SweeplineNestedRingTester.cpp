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


#include "../../headers/opValid.h"
#include "stdio.h"
#include "../../headers/util.h"

namespace geos {

SweeplineNestedRingTester::SweeplineNestedRingTester(GeometryGraph *newGraph) {
	graph=newGraph;
	rings=new vector<LinearRing*>();
	totalEnv=new Envelope();
	sweepLine=new SweepLineIndex();
	cga=new CGAlgorithms();
}

SweeplineNestedRingTester::~SweeplineNestedRingTester() {
	delete rings;
	delete totalEnv;
	delete sweepLine;
	delete cga;
}

SweeplineNestedRingTester::OverlapAction::OverlapAction(SweeplineNestedRingTester *p) {
	isNonNested=true;
	parent=p;
}
void SweeplineNestedRingTester::OverlapAction::overlap(SweepLineInterval *s0, SweepLineInterval *s1) {
	LinearRing *innerRing=(LinearRing*) s0->getItem();
	LinearRing *searchRing=(LinearRing*) s1->getItem();
	if (innerRing==searchRing) return;
	if (parent->isInside(innerRing,searchRing))
		isNonNested=false;
};


Coordinate& SweeplineNestedRingTester::getNestedPoint() {
	return nestedPt;
}

void SweeplineNestedRingTester::add(LinearRing *ring) {
	rings->push_back(ring);
}

bool SweeplineNestedRingTester::isNonNested() {
	buildIndex();
	OverlapAction *action=new OverlapAction(this);
	sweepLine->computeOverlaps(action);
	return action->isNonNested;
}

void SweeplineNestedRingTester::buildIndex() {
	sweepLine=new SweepLineIndex();
	for(int i=0;i<(int)rings->size();i++) {
		LinearRing *ring=(*rings)[i];
		Envelope *env=ring->getEnvelopeInternal();
		SweepLineInterval *sweepInt=new SweepLineInterval(env->getMinX(),env->getMaxX(),ring);
		sweepLine->add(sweepInt);
	}
}

bool SweeplineNestedRingTester::isInside(LinearRing *innerRing,LinearRing *searchRing) {
	CoordinateList *innerRingPts=innerRing->getCoordinates();
	CoordinateList *searchRingPts=searchRing->getCoordinates();

	if (!innerRing->getEnvelopeInternal()->intersects(searchRing->getEnvelopeInternal()))
		return false;
	const Coordinate& innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
	Assert::isTrue(!(innerRingPt==Coordinate::getNull()), "Unable to find a ring point not a node of the search ring");

	bool isInside=cga->isPointInRing(innerRingPt,searchRingPts);
	if (isInside) {
		nestedPt=innerRingPt;
		return true;
	}
	return false;
}
}

