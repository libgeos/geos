#include "../../headers/opValid.h"
#include "stdio.h"
#include "../../headers/util.h"

namespace geos {

QuadtreeNestedRingTester::QuadtreeNestedRingTester(GeometryGraph *newGraph) {
	graph=newGraph;
	rings=new vector<LinearRing*>();
	totalEnv=new Envelope();
	qt=NULL;
	cga=new RobustCGAlgorithms();
}

QuadtreeNestedRingTester::~QuadtreeNestedRingTester() {
	delete rings;
	delete totalEnv;
	delete cga;
	delete qt;
}

Coordinate& QuadtreeNestedRingTester::getNestedPoint() {
	return nestedPt;
}

void QuadtreeNestedRingTester::add(LinearRing *ring) {
	rings->push_back(ring);
	Envelope *envi=ring->getEnvelopeInternal();
	totalEnv->expandToInclude(envi);
	delete envi;
}

bool QuadtreeNestedRingTester::isNonNested() {
	buildQuadtree();
	for(int i=0;i<(int)rings->size();i++) {
		LinearRing *innerRing=(*rings)[i];
		CoordinateList *innerRingPts=innerRing->getCoordinates();
		Envelope *envi=innerRing->getEnvelopeInternal();
		vector<void*> *results=qt->query(envi);
		delete envi;
		for(int j=0;j<(int)results->size();j++) {
			LinearRing *searchRing=(LinearRing*)(*results)[j];
			CoordinateList *searchRingPts=searchRing->getCoordinates();
			if (innerRing==searchRing)
				continue;
			Envelope *e1=innerRing->getEnvelopeInternal();
			Envelope *e2=searchRing->getEnvelopeInternal();
			if (!e1->intersects(e2)) {
				delete e1;
				delete e2;
				continue;
			}
			delete e1;
			delete e2;
			Coordinate& innerRingPt=IsValidOp::findPtNotNode(innerRingPts,searchRing,graph);
			Assert::isTrue(!(innerRingPt==Coordinate::getNull()),"Unable to find a ring point not a node of the search ring");
			//Coordinate innerRingPt = innerRingPts[0];
			bool isInside=cga->isPointInRing(innerRingPt,searchRingPts);
			if (isInside) {
				nestedPt=innerRingPt;
				delete results;
				return false;
			}
		}
		delete results;
	}
	return true;
}

void QuadtreeNestedRingTester::buildQuadtree() {
	qt=new Quadtree();
	for(int i=0;i<(int)rings->size();i++) {
		LinearRing *ring=(*rings)[i];
		Envelope *env=ring->getEnvelopeInternal();
		qt->insert(env,ring);
		delete env;
	}
}
}

