#include "opOverlay.h"
#include "stdio.h"

EdgeSetNoder::EdgeSetNoder(LineIntersector *newLi) {
	li=newLi;
	inputEdges=new vector<Edge*>();
}

EdgeSetNoder::~EdgeSetNoder() {
	delete inputEdges;
}

void EdgeSetNoder::addEdges(vector<Edge*> *edges){
	inputEdges->insert(inputEdges->end(),edges->begin(),edges->end());
}

vector<Edge*>* EdgeSetNoder::getNodedEdges() {
	EdgeSetIntersector *esi=new SimpleMCSweepLineIntersector();
	SegmentIntersector *si=new SegmentIntersector(li,true,false);
	esi->computeIntersections(inputEdges,si);
	//Debug.println("has proper int = " + si.hasProperIntersection());
	vector<Edge*> *splitEdges=new vector<Edge*>();
	for(int i=0;i<(int)inputEdges->size();i++) {
		Edge* e=(*inputEdges)[i];
		e->getEdgeIntersectionList()->addSplitEdges(splitEdges);
	}
	return splitEdges;
}