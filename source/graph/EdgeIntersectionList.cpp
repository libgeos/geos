#include "graph.h"

EdgeIntersectionList::EdgeIntersectionList(Edge *newEdge) {
	list=new vector<EdgeIntersection*>();
	edge=newEdge;
}

EdgeIntersectionList::~EdgeIntersectionList() {
//	delete edge;
	delete list;
}

EdgeIntersection* EdgeIntersectionList::add(Coordinate& coord, int segmentIndex, double dist){
	vector<EdgeIntersection *>::iterator insertIt=list->begin();
	bool isInList=findInsertionPoint(segmentIndex,dist,&insertIt);
	EdgeIntersection *ei;
	if (!isInList) {
		ei=new EdgeIntersection(coord,segmentIndex,dist);
		list->insert(insertIt,ei);
	} else
		ei=*insertIt;
	return ei;
}

vector<EdgeIntersection*>::iterator EdgeIntersectionList::iterator(){
	return list->begin();
}

bool EdgeIntersectionList::isEmpty() {
	return list->empty();
}

bool EdgeIntersectionList::findInsertionPoint(int segmentIndex, double dist,vector<EdgeIntersection*>::iterator *insertIt){
	vector<EdgeIntersection *>::iterator findIt=list->begin();
	bool found=false;
	while(findIt<list->end()) {
		EdgeIntersection *ei=*findIt;
		findIt++;
		int compare=ei->compare(segmentIndex, dist);
		// intersection found - insertIt.next() will retrieve it
		if (compare==0) return true;
		// this ei is past the intersection location, so intersection was not found
		if (compare>0) return false;
		// this ei was before the intersection point, so move to next
		(*insertIt)++;
	}
	return false;
}

bool EdgeIntersectionList::isIntersection(Coordinate& pt){
	vector<EdgeIntersection *>::iterator it;
	for (it=list->begin();it<list->end();it++) {
		EdgeIntersection *ei=*it;
		if (ei->coord==pt)
			return true;
	}
	return false;
}

void EdgeIntersectionList::addEndpoints(){
	int maxSegIndex=edge->pts->getSize()-1;
	add(edge->pts->getAt(0), 0, 0.0);
	add(edge->pts->getAt(maxSegIndex), maxSegIndex, 0.0);
}

void EdgeIntersectionList::addSplitEdges(vector<Edge*> *edgeList) {
	// ensure that the list has entries for the first and last point of the edge
	addEndpoints();
	vector<EdgeIntersection *>::iterator it=list->begin();
	// there should always be at least two entries in the list
	EdgeIntersection *eiPrev=*it;
	it++;
	while (it<list->end()) {
		EdgeIntersection *ei=*it;
		Edge *newEdge=createSplitEdge(eiPrev,ei);
		edgeList->push_back(newEdge);
		eiPrev=ei;
		it++;
	}
}

Edge* EdgeIntersectionList::createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1) {
	int npts=ei1->segmentIndex-ei0->segmentIndex+2;
	Coordinate& lastSegStartPt=edge->pts->getAt(ei1->segmentIndex);
	// if the last intersection point is not equal to the its segment start pt,
	// add it to the points list as well.
	// (This check is needed because the distance metric is not totally reliable!)
	bool useIntPt1=ei1->dist>0.0 || !(ei1->coord==lastSegStartPt);
	if (!useIntPt1) {
		npts--;
	}
	CoordinateList* pts=CoordinateListFactory::internalFactory->createCoordinateList(npts);
	int ipt=0;
	pts->setAt(*(new Coordinate(ei0->coord)),ipt++);
	for(int i=ei0->segmentIndex+1; i<=ei1->segmentIndex;i++) {
		pts->setAt(edge->pts->getAt(i),ipt++);
	}
	if (useIntPt1) pts->setAt(ei1->coord,ipt);
	return new Edge(pts,edge->getLabel());
}

string EdgeIntersectionList::print(){
	string out="Intersections: ";
	vector<EdgeIntersection *>::iterator it;
	for (it=list->begin();it<list->end();it++) {
		EdgeIntersection *ei=*it;
		out+=ei->print();
	}
	return out;
}

