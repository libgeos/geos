#include "opValid.h"
#include "stdio.h"
#include "util.h"

CGAlgorithms* QuadtreeNestedRingTester::cga=new RobustCGAlgorithms();

QuadtreeNestedRingTester::QuadtreeNestedRingTester(GeometryGraph *newGraph) {
	graph=newGraph;
	rings=new vector<LinearRing*>();
	totalEnv=new Envelope();
	qt=new Quadtree(totalEnv);
}

QuadtreeNestedRingTester::~QuadtreeNestedRingTester() {
	delete rings;
	delete totalEnv;
	delete qt;
}

Coordinate& QuadtreeNestedRingTester::getNestedPoint() {
	return nestedPt;
}

void QuadtreeNestedRingTester::add(LinearRing *ring) {
	rings->push_back(ring);
	totalEnv->expandToInclude(ring->getEnvelopeInternal());
}

bool QuadtreeNestedRingTester::isNonNested() {
	buildQuadtree();
	for(int i=0;i<(int)rings->size();i++) {
		LinearRing *innerRing=(*rings)[i];
		CoordinateList *innerRingPts=innerRing->getCoordinates();
		vector<void*> *results=qt->query(innerRing->getEnvelopeInternal());
		for(int j=0;j<(int)results->size();j++) {
			LinearRing *searchRing=(LinearRing*)(*results)[j];
			CoordinateList *searchRingPts=searchRing->getCoordinates();
			if (innerRing==searchRing)
				continue;
			if (!innerRing->getEnvelopeInternal()->overlaps(searchRing->getEnvelopeInternal()))
				continue;
			Coordinate& innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
			Assert::isTrue(!(innerRingPt==Coordinate::getNull()),"Unable to find a ring point not a node of the search ring");
			//Coordinate innerRingPt = innerRingPts[0];
			bool isInside=cga->isPointInRing(innerRingPt,searchRingPts);
			if (isInside) {
				nestedPt=innerRingPt;
				return false;
			}
		}
	}
	return true;
}

void QuadtreeNestedRingTester::buildQuadtree() {
	qt=new Quadtree(totalEnv);
	for(int i=0;i<(int)rings->size();i++) {
		LinearRing *ring=(*rings)[i];
		Envelope *env=ring->getEnvelopeInternal();
		qt->insert(env,ring);
	}
}