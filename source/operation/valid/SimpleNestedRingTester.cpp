#include "../../headers/opValid.h"
#include "../../headers/util.h"
#include "stdio.h"

namespace geos {

SimpleNestedRingTester::SimpleNestedRingTester(GeometryGraph *newGraph){
	graph=newGraph;
	rings=new vector<LinearRing*>();
	cga=new RobustCGAlgorithms();
}

SimpleNestedRingTester::~SimpleNestedRingTester(){
	delete rings;
	delete cga;
}


void SimpleNestedRingTester::add(LinearRing *ring){
	rings->push_back(ring);
}

Coordinate& SimpleNestedRingTester::getNestedPoint() {
	return nestedPt;
}

bool SimpleNestedRingTester::isNonNested(){
	for(int i=0;i< (int)rings->size(); i++) {
		LinearRing* innerRing=(*rings)[i];
		CoordinateList *innerRingPts=innerRing->getCoordinates();
		for(int j= 0;j<(int)rings->size(); j++) {
			LinearRing* searchRing=(*rings)[j];
			CoordinateList *searchRingPts=searchRing->getCoordinates();
			if (innerRing==searchRing)
				continue;
			if (!innerRing->getEnvelopeInternal()->intersects(searchRing->getEnvelopeInternal()))
				continue;
			Coordinate& innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
			Assert::isTrue(innerRingPt==Coordinate::getNull(), "Unable to find a ring point not a node of the search ring");

			bool isInside=cga->isPointInRing(innerRingPt,searchRingPts);
			if (isInside) {
				nestedPt=innerRingPt;
				return false;
			}
		}
	}
	return true;
}
}

