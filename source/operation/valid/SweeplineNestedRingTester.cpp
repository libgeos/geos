#include "../../headers/opValid.h"
#include "stdio.h"
#include "../../headers/util.h"

namespace geos {

SweeplineNestedRingTester::SweeplineNestedRingTester(GeometryGraph *newGraph) {
	graph=newGraph;
	rings=new vector<LinearRing*>();
	totalEnv=new Envelope();
	sweepLine=new SweepLineIndex();
	cga=new RobustCGAlgorithms();
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
	Coordinate& innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
	Assert::isTrue(!(innerRingPt==Coordinate::getNull()), "Unable to find a ring point not a node of the search ring");

	bool isInside=cga->isPointInRing(innerRingPt,searchRingPts);
	if (isInside) {
		nestedPt=innerRingPt;
		return true;
	}
	return false;
}
}