#include "operation.h"
#include "stdio.h"
#include <set>

IsSimpleOp::IsSimpleOp(){}

bool IsSimpleOp::isSimple(LineString *geom){
	return isSimpleLinearGeometry(geom);
}

bool IsSimpleOp::isSimple(MultiLineString *geom){
	return isSimpleLinearGeometry(geom);
}

/**
* A MultiPoint is simple iff it has no repeated points
*/
bool IsSimpleOp::isSimple(MultiPoint *mp) {
	if (mp->isEmpty()) return true;
	set<Coordinate,CoordLT> *points=new set<Coordinate,CoordLT>();
	for(int i=0;i<mp->getNumGeometries();i++) {
		Point *pt=(Point*) mp->getGeometryN(i);
		Coordinate *p=pt->getCoordinate();
		if (points->find(*p)!=points->end()) {
			delete points;
			return false;
		}
		points->insert(*p);
	}
	delete points;
	return true;
}

bool IsSimpleOp::isSimpleLinearGeometry(Geometry *geom){
	if (geom->isEmpty()) return true;
	GeometryGraph *graph=new GeometryGraph(0,geom);
	SegmentIntersector *si=graph->computeSelfNodes(graph->li);
	// if no self-intersection, must be simple
	if (!si->hasIntersection()) return true;
	if (si->hasProperIntersection()) return false;
	if (hasNonEndpointIntersection(graph)) return false;
	if (hasClosedEndpointIntersection(graph)) return false;
	return true;
}

/**
* For all edges, check if there are any intersections which are NOT at an endpoint.
* The Geometry is not simple if there are intersections not at endpoints.
*/
bool IsSimpleOp::hasNonEndpointIntersection(GeometryGraph *graph) {
	vector<Edge*> *edges=graph->getEdges();
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int maxSegmentIndex=e->getMaximumSegmentIndex();
		EdgeIntersectionList *eiL=e->getEdgeIntersectionList();
		vector<EdgeIntersection*> intList(eiL->list);
		for(vector<EdgeIntersection*>::iterator eiIt=intList.begin();eiIt<intList.end();eiIt++) {
			EdgeIntersection *ei=*eiIt;
			if (!ei->isEndPoint(maxSegmentIndex))
				return true;
		}
	}
	return false;
}

/**
* Test that no edge intersection is the
* endpoint of a closed line.  To check this we compute the
* degree of each endpoint. The degree of endpoints of closed lines
* must be exactly 2.
*/
bool IsSimpleOp::hasClosedEndpointIntersection(GeometryGraph *graph) {
	map<Coordinate,EndpointInfo*,CoordLT> *endPoints=new map<Coordinate,EndpointInfo*,CoordLT>();
	vector<Edge*> *edges=graph->getEdges();
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int maxSegmentIndex=e->getMaximumSegmentIndex();
		bool isClosed=e->isClosed();
		Coordinate p0(e->getCoordinate(0));
		addEndpoint(endPoints,p0,isClosed);
		Coordinate p1(e->getCoordinate(e->getNumPoints()-1));
		addEndpoint(endPoints,p1,isClosed);
	}
	map<Coordinate,EndpointInfo*,CoordLT>::iterator it=endPoints->begin();
	for (;it!=endPoints->end();it++) {
		EndpointInfo *eiInfo=it->second;
		if (eiInfo->isClosed && eiInfo->degree!=2) {
			delete endPoints;
            return true;
		}
	}
	delete endPoints;
	return false;
}

/**
* Add an endpoint to the map, creating an entry for it if none exists
*/
void IsSimpleOp::addEndpoint(map<Coordinate,EndpointInfo*,CoordLT> *endPoints,Coordinate p,bool isClosed) {
	EndpointInfo *eiInfo=endPoints->find(p)->second;
	if (eiInfo==NULL) {
		eiInfo=new EndpointInfo(p);
		(*endPoints)[p]=eiInfo;
	}
	eiInfo->addEndpoint(isClosed);
}

EndpointInfo::EndpointInfo(Coordinate newPt) {
	pt.setCoordinate(newPt);
	isClosed=false;
	degree=0;
}

void EndpointInfo::addEndpoint(bool newIsClosed) {
	degree++;
	isClosed|=newIsClosed;
}
