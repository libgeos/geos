#include "graph.h"
#include <typeinfo>
#include "util.h"

/**
* This method implements the Boundary Determination Rule
* for determining whether
* a component (node or edge) that appears multiple times in elements
* of a MultiGeometry is in the boundary or the interior of the Geometry
* <br>
* The SFS uses the "Mod-2 Rule", which this function implements
* <br>
* An alternative (and possibly more intuitive) rule would be
* the "At Most One Rule":
*    isInBoundary = (componentCount == 1)
*/
bool GeometryGraph::isInBoundary(int boundaryCount){
	// the "Mod-2 Rule"
	return boundaryCount%2==1;
}

int GeometryGraph::determineBoundary(int boundaryCount){
	return isInBoundary(boundaryCount)?Location::BOUNDARY : Location::INTERIOR;
}

GeometryGraph::GeometryGraph():PlanarGraph(){
	precisionModel=NULL;
//	lineEdgeMap=new map<LineString*,Edge*,LineStringLT>();
	newPM=NULL;
	useBoundaryDeterminationRule=false;
	boundaryNodes=NULL;
}

GeometryGraph::GeometryGraph(int newArgIndex, Geometry *newParentGeom):PlanarGraph() {
	boundaryNodes=NULL;
	precisionModel=NULL;
	newPM=NULL;
	useBoundaryDeterminationRule=false;
	argIndex=newArgIndex;
	parentGeom=newParentGeom;
	if (parentGeom!=NULL) {
		precisionModel=&(parentGeom->getPrecisionModel());
		SRID=parentGeom->getSRID();
		add(parentGeom);
	}
}

EdgeSetIntersector* GeometryGraph::createEdgeSetIntersector() {
	// various options for computing intersections, from slowest to fastest

	//private EdgeSetIntersector esi = new SimpleEdgeSetIntersector();
	//private EdgeSetIntersector esi = new MonotoneChainIntersector();
	//private EdgeSetIntersector esi = new NonReversingChainIntersector();
	//private EdgeSetIntersector esi = new SimpleSweepLineIntersector();
	//private EdgeSetIntersector esi = new MCSweepLineIntersector();

	//return new SimpleEdgeSetIntersector();
	return new SimpleMCSweepLineIntersector();
}

/**
* This constructor is used by clients that wish to add Edges explicitly,
* rather than adding a Geometry.  (An example is BufferOp).
*/
GeometryGraph::GeometryGraph(int newArgIndex, PrecisionModel *newPrecisionModel, int newSRID):PlanarGraph(){
	boundaryNodes=NULL;
	GeometryGraph(newArgIndex,NULL);
	precisionModel=newPrecisionModel;
	SRID=newSRID;
}

PrecisionModel* GeometryGraph::getPrecisionModel(){
	return precisionModel;
}

int GeometryGraph::getSRID() {
	return SRID;
}

Geometry* GeometryGraph::getGeometry() {
	return parentGeom;
}

vector<Node*>* GeometryGraph::getBoundaryNodes() {
	if (boundaryNodes==NULL)
		boundaryNodes=new vector<Node*>(nodes->getBoundaryNodes(argIndex));
	return boundaryNodes;
}

CoordinateList GeometryGraph::getBoundaryPoints() {
	vector<Node*> coll(*getBoundaryNodes());
	CoordinateList pts((int)coll.size());
	int i=0;
	for (vector<Node*>::iterator it=coll.begin();it<coll.end();it++) {
		Node *node=*it;
		pts.setAt(node->getCoordinate(),i++);
	}
	return pts;
}

Edge* GeometryGraph::findEdge(LineString *line){
	return lineEdgeMap.find(line)->second;
}

void GeometryGraph::computeSplitEdges(vector<Edge*> *edgelist) {
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		e->eiList->addSplitEdges(edgelist);
	}
}

void GeometryGraph::add(Geometry *g) {
	if (g->isEmpty()) return;
	// check if this Geometry should obey the Boundary Determination Rule
	// all collections except MultiPolygons obey the rule
	if (typeid(*g)==typeid(GeometryCollection)
		&& !(typeid(*g)==typeid(MultiPolygon)))
			useBoundaryDeterminationRule=true;
	if (typeid(*g)==typeid(Polygon))
		addPolygon((Polygon*) g);
	// LineString also handles LinearRings
	else if (typeid(*g)==typeid(LineString))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(Point))
		addPoint((Point*) g);
	else if (typeid(*g)==typeid(MultiPoint))
		addCollection((MultiPoint*) g);
	else if (typeid(*g)==typeid(MultiLineString))
		addCollection((MultiLineString*) g);
	else if (typeid(*g)==typeid(MultiPolygon))
		addCollection((MultiPolygon*) g);
	else if (typeid(*g)==typeid(GeometryCollection))
		addCollection((GeometryCollection*) g);
	else {
		string out="UnsupportedOperationException: ";
		out+=typeid(*g).name();
		throw out;
	}
}

void GeometryGraph::addCollection(GeometryCollection *gc) {
	for (int i=0;i<gc->getNumGeometries();i++) {
		Geometry *g=gc->getGeometryN(i);
		add(g);
	}
}

/**
* Add a Point to the graph.
*/
void GeometryGraph::addPoint(Point *p){
	Coordinate coord(p->getCoordinate());
	insertPoint(argIndex,coord,Location::INTERIOR);
}

/**
* The left and right topological location arguments assume that the ring is oriented CW.
* If the ring is in the opposite orientation,
* the left and right locations must be interchanged.
*/
void GeometryGraph::addPolygonRing(LinearRing *lr, int cwLeft, int cwRight) {
	CoordinateList coord(lr->getCoordinates());
	int left=cwLeft;
	int right=cwRight;
	if (cga->isCCW(coord)) {
		left=cwRight;
		right=cwLeft;
	}
	Edge *e=new Edge(coord,new Label(argIndex,Location::BOUNDARY,left,right));
	lineEdgeMap[lr]=e;
	insertEdge(e);
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex,coord.getAt(0), Location::BOUNDARY);
}

void GeometryGraph::addPolygon(Polygon *p){
	addPolygonRing((LinearRing*) p->getExteriorRing(),Location::EXTERIOR,Location::INTERIOR);
	for (int i=0;i<p->getNumInteriorRing();i++) {
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CW)
		addPolygonRing((LinearRing*) p->getInteriorRingN(i),Location::INTERIOR,Location::EXTERIOR);
	}
}

void GeometryGraph::addLineString(LineString *line){
	CoordinateList coord(line->getCoordinates());
	// add the edge for the LineString
	// line edges do not have locations for their left and right sides
	Edge *e=new Edge(coord,new Label(argIndex,Location::INTERIOR));
	lineEdgeMap[line]=e;
	insertEdge(e);
	/**
	* Add the boundary points of the LineString, if any.
	* Even if the LineString is closed, add both points as if they were endpoints.
	* This allows for the case that the node already exists and is a boundary point.
	*/
	Assert::isTrue(coord.getSize()>= 2,"found LineString with single point");
	insertBoundaryPoint(argIndex,coord.getAt(0));
	insertBoundaryPoint(argIndex,coord.getAt(coord.getSize()-1));
}

/**
* Add an Edge computed externally.  The label on the Edge is assumed
* to be correct.
*/
void GeometryGraph::addEdge(Edge *e) {
	insertEdge(e);
	CoordinateList coord(e->getCoordinates());
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex,coord.getAt(0),Location::BOUNDARY);
	insertPoint(argIndex,coord.getAt(coord.getSize()-1),Location::BOUNDARY);
}

/**
* Add a point computed externally.  The point is assumed to be a
* Point Geometry part, which has a location of INTERIOR.
*/
void GeometryGraph::addPoint(Coordinate pt) {
	insertPoint(argIndex,pt,Location::INTERIOR);
}

SegmentIntersector* GeometryGraph::computeSelfNodes(LineIntersector *li){
	SegmentIntersector *si=new SegmentIntersector(li,true,false);
	//EdgeSetIntersector esi = new MCQuadIntersector();
    EdgeSetIntersector *esi=createEdgeSetIntersector();
	esi->computeIntersections(edges,si);
	//System.out.println("SegmentIntersector # tests = " + si.numTests);
	addSelfIntersectionNodes(argIndex);
	return si;
}

SegmentIntersector* GeometryGraph::computeEdgeIntersections(GeometryGraph *g,
													LineIntersector *li,
													bool includeProper){
	SegmentIntersector *si=new SegmentIntersector(li,includeProper,true);
	si->setBoundaryNodes(getBoundaryNodes(),g->getBoundaryNodes());
	EdgeSetIntersector *esi=createEdgeSetIntersector();
	esi->computeIntersections(edges,g->edges,si);
	return si;
}

void GeometryGraph::insertPoint(int argIndex, Coordinate coord, int onLocation){
	Node *n=nodes->addNode(coord);
	Label *lbl=n->getLabel();
	if (lbl==NULL) {
		n->setLabel(argIndex,onLocation);
	} else
		lbl->setLocation(argIndex,onLocation);
}

/**
* Adds points using the mod-2 rule of SFS.  This is used to add the boundary
* points of dim-1 geometries (Curves/MultiCurves).  According to the SFS,
* an endpoint of a Curve is on the boundary
* iff if it is in the boundaries of an odd number of Geometries
*/
void GeometryGraph::insertBoundaryPoint(int argIndex,Coordinate coord){
	Node *n=nodes->addNode(coord);
	Label *lbl=n->getLabel();
	// the new point to insert is on a boundary
	int boundaryCount=1;
	// determine the current location for the point (if any)
	int loc=Location::UNDEF;
	if (lbl!=NULL) loc=lbl->getLocation(argIndex,Position::ON);
	if (loc==Location::BOUNDARY) boundaryCount++;
	// determine the boundary status of the point according to the Boundary Determination Rule
	int newLoc=determineBoundary(boundaryCount);
	lbl->setLocation(argIndex,newLoc);
}

void GeometryGraph::addSelfIntersectionNodes(int argIndex){
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int eLoc=e->getLabel()->getLocation(argIndex);
		vector<EdgeIntersection*> eil=e->eiList->list;
		for (vector<EdgeIntersection*>::iterator eiIt=eil.begin();eiIt<eil.end();eiIt++) {
			EdgeIntersection *ei=*eiIt;
			addSelfIntersectionNode(argIndex,ei->coord,eLoc);
		}
	}
}

/**
* Add a node for a self-intersection.
* If the node is a potential boundary node (e.g. came from an edge which
* is a boundary) then insert it as a potential boundary node.
* Otherwise, just add it as a regular node.
*/
void GeometryGraph::addSelfIntersectionNode(int argIndex,Coordinate coord,int loc){
	// if this node is already a boundary node, don't change it
	if (isBoundaryNode(argIndex,coord)) return;
	if (loc==Location::BOUNDARY && useBoundaryDeterminationRule)
		insertBoundaryPoint(argIndex,coord);
	else
		insertPoint(argIndex,coord,loc);
}

vector<Edge*> *GeometryGraph::getEdges() {
	return edges;
}

