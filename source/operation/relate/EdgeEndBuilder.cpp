#include "opRelate.h"
#include "stdio.h"

EdgeEndBuilder::EdgeEndBuilder() {}

vector<EdgeEnd*> *EdgeEndBuilder::computeEdgeEnds(vector<Edge*> *edges) {
	vector<EdgeEnd*> *l=new vector<EdgeEnd*>();
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		computeEdgeEnds(e,l);
	}
	return l;
}

/**
* Creates stub edges for all the intersections in this
* Edge (if any) and inserts them into the graph.
*/
void EdgeEndBuilder::computeEdgeEnds(Edge *edge,vector<EdgeEnd*> *l) {
	EdgeIntersectionList *eiList=edge->getEdgeIntersectionList();
	//Debug.print(eiList);
	// ensure that the list has entries for the first and last point of the edge
	if (!edge->isClosed()) eiList->addEndpoints();
	vector<EdgeIntersection*>::iterator it=eiList->list.begin();
	EdgeIntersection *eiPrev=NULL;
	EdgeIntersection *eiCurr=NULL;
	// no intersections, so there is nothing to do
	if (!(it<eiList->list.end())) return;
	EdgeIntersection *eiNext=*it;
	it++;
	do {
		eiPrev=eiCurr;
		eiCurr=eiNext;
		eiNext=NULL;
		if (it<eiList->list.end()) {
			eiNext=*it;
			it++;
		}
		if (eiCurr!=NULL) {
			createEdgeEndForPrev(edge,l,eiCurr,eiPrev);
			createEdgeEndForNext(edge,l,eiCurr,eiNext);
		}
	} while (eiCurr!=NULL);
}

/**
* Create a EdgeStub for the edge before the intersection eiCurr.
* The previous intersection is provided
* in case it is the endpoint for the stub edge.
* Otherwise, the previous point from the parent edge will be the endpoint.
* <br>
* eiCurr will always be an EdgeIntersection, but eiPrev may be null.
*/
void EdgeEndBuilder::createEdgeEndForPrev(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiPrev) {
	int iPrev=eiCurr->segmentIndex;
	if (eiCurr->dist==0.0) {
		// if at the start of the edge there is no previous edge
		if (iPrev==0) return;
		iPrev--;
	}
	Coordinate pPrev(edge->getCoordinate(iPrev));
	// if prev intersection is past the previous vertex, use it instead
	if (eiPrev!=NULL && eiPrev->segmentIndex>=iPrev)
		pPrev.setCoordinate(eiPrev->coord);
	Label *label=new Label(*(edge->getLabel()));
	// since edgeStub is oriented opposite to it's parent edge, have to flip sides for edge label
	label->flip();
	EdgeEnd *e=new EdgeEnd(edge,eiCurr->coord,pPrev,label);
	//e.print(System.out);  System.out.println();
	l->push_back(e);
}

/**
* Create a StubEdge for the edge after the intersection eiCurr.
* The next intersection is provided
* in case it is the endpoint for the stub edge.
* Otherwise, the next point from the parent edge will be the endpoint.
* <br>
* eiCurr will always be an EdgeIntersection, but eiNext may be null.
*/
void EdgeEndBuilder::createEdgeEndForNext(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiNext) {
	int iNext=eiCurr->segmentIndex + 1;
	// if there is no next edge there is nothing to do
	if (iNext>=edge->getNumPoints() && eiNext==NULL) return;
	Coordinate pNext(edge->getCoordinate(iNext));
	// if the next intersection is in the same segment as the current, use it as the endpoint
	if (eiNext!=NULL && eiNext->segmentIndex==eiCurr->segmentIndex)
		pNext.setCoordinate(eiNext->coord);
	EdgeEnd *e=new EdgeEnd(edge,eiCurr->coord,pNext,new Label(*(edge->getLabel())));
	//Debug.println(e);
	l->push_back(e);
}
