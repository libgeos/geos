#include "../../headers/opBuffer.h"
#include <algorithm>

namespace geos {

Geometry* BufferOp::bufferOp(Geometry *g, double distance){
	BufferOp *gBuf=new BufferOp(g);
	Geometry *geomBuf=gBuf->getResultGeometry(distance);
	delete gBuf;
	return geomBuf;
}

Geometry* BufferOp::bufferOp(Geometry *g, double distance, int quadrantSegments){
	BufferOp *gBuf=new BufferOp(g);
	Geometry *geomBuf=gBuf->getResultGeometry(distance, quadrantSegments);
	return geomBuf;
}

/**
*Compute the change in depth as an edge is crossed from R to L
*/
int BufferOp::depthDelta(Label *label){
	int lLoc=label->getLocation(0,Position::LEFT);
	int rLoc=label->getLocation(0,Position::RIGHT);
	if (lLoc==Location::INTERIOR && rLoc==Location::EXTERIOR)
		return 1;
	else if (lLoc==Location::EXTERIOR && rLoc==Location::INTERIOR)
		return -1;
	return 0;
}

BufferOp::BufferOp(Geometry *g0): GeometryGraphOperation(g0) {
	resultGeom=NULL;
	edgeList=new EdgeList();
	graph=new PlanarGraph(new OverlayNodeFactory());
	geomFact=new GeometryFactory(g0->getPrecisionModel(),g0->getSRID());
}

BufferOp::~BufferOp(){
	delete edgeList;
	delete graph;
	delete geomFact;
}

Geometry* BufferOp::getResultGeometry(double distance) {
	computeBuffer(distance,BufferLineBuilder::DEFAULT_QUADRANT_SEGMENTS);
	return resultGeom;
}

Geometry* BufferOp::getResultGeometry(double distance, int quadrantSegments) {
	computeBuffer(distance, quadrantSegments);
	return resultGeom;
}

void BufferOp::computeBuffer(double distance, int quadrantSegments) {
	BufferEdgeBuilder *bufEdgeBuilder=new BufferEdgeBuilder(cga,li,distance,resultPrecisionModel,quadrantSegments);
	vector<Edge*> *bufferEdgeList=bufEdgeBuilder->getEdges(getArgGeometry(0));
	vector<Edge*> *nodedEdges=nodeEdges(bufferEdgeList);
	for(int i=0;i<(int)nodedEdges->size();i++) {
		Edge *e=(*nodedEdges)[i];
		insertEdge(e);
	}
	replaceCollapsedEdges();
	graph->addEdges(edgeList);

	vector<BufferSubgraph*> *subgraphList=createSubgraphs();
	PolygonBuilder *polyBuilder=new PolygonBuilder(geomFact,cga);
	buildSubgraphs(subgraphList,polyBuilder);
	vector<Polygon*> *resultPolyList=polyBuilder->getPolygons();
	resultGeom=computeGeometry(resultPolyList);
	//computeBufferLine(graph);
	delete bufEdgeBuilder;
	delete polyBuilder;
	delete resultPolyList;
	for(int i=0;i<(int)subgraphList->size();i++) {
		delete (*subgraphList)[i];
	}
	delete subgraphList;
	delete nodedEdges;
}

/**
*Use a GeometryGraph to node the created edges,
*and create split edges between the nodes
*/
vector<Edge*>* BufferOp::nodeEdges(vector<Edge*> *edges){
	// intersect edges again to ensure they are noded correctly
	GeometryGraph *ggraph=new GeometryGraph(0,geomFact->getPrecisionModel(),0);
	for (int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		ggraph->addEdge(e);
	}
	SegmentIntersector *si=ggraph->computeSelfNodes(li, false);
	/*
	if (si.hasProperIntersection())
	Debug.println("proper intersection found");
	else
	Debug.println("no proper intersection found");
	*/
	vector<Edge*> *newEdges=new vector<Edge*>();
	ggraph->computeSplitEdges(newEdges);
	delete si;
	delete ggraph;
	return newEdges;
}
/**
*Inserted edges are checked identical edge already exists.
*If so, the edge is not inserted, but its label is merged
*with the existing edge.
*/
void BufferOp::insertEdge(Edge *e){
	//Debug.println(e);
	int foundIndex=edgeList->findEdgeIndex(e);
	// If an identical edge already exists, simply update its label
	if (foundIndex>=0) {
		Edge *existingEdge=(*edgeList)[foundIndex];
		Label *existingLabel=existingEdge->getLabel();
		Label *labelToMerge=e->getLabel();
		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (!existingEdge->isPointwiseEqual(e)) {
			labelToMerge=new Label(e->getLabel());
			labelToMerge->flip();
		}
		existingLabel->merge(labelToMerge);
		// compute new depth delta of sum of edges
		int mergeDelta=depthDelta(labelToMerge);
		int existingDelta=existingEdge->getDepthDelta();
		int newDelta=existingDelta+mergeDelta;
		existingEdge->setDepthDelta(newDelta);
		checkDimensionalCollapse(labelToMerge,existingLabel);
		//Debug.print("new edge "); Debug.println(e);
		//Debug.print("existing "); Debug.println(existingEdge);
	} else {   // no matching existing edge was found
		// add this new edge to the list of edges in this graph
		//e.setName(name+edges.size());
		edgeList->push_back(e);
		e->setDepthDelta(depthDelta(e->getLabel()));
	}
}

/**
*If either of the GeometryLocations for the existing label is
*exactly opposite to the one in the labelToMerge,
*this indicates a dimensional collapse has happened.
*In this case, convert the label for that Geometry to a Line label
*/
void BufferOp::checkDimensionalCollapse(Label *labelToMerge,Label *existingLabel){
	if (existingLabel->isArea() && labelToMerge->isArea()) {
		for (int i=0;i<2;i++) {
			if (!labelToMerge->isNull(i)
				&&  labelToMerge->getLocation(i,Position::LEFT)==existingLabel->getLocation(i,Position::RIGHT)
				&&  labelToMerge->getLocation(i,Position::RIGHT)==existingLabel->getLocation(i,Position::LEFT)) {
					existingLabel->toLine(i);
			}
		}
	}
}

/**
*If collapsed edges are found, replace them with a new edge which is a L edge
*/
void BufferOp::replaceCollapsedEdges() {
	vector<Edge*> *newEdges=new vector<Edge*>();
	for(int i=0;i<(int)edgeList->size();i++) {
		Edge *e=(*edgeList)[i];
		if (e->isCollapsed()) {
			//Debug.print(e);
			edgeList->erase(edgeList->begin()+i);
			newEdges->push_back(e->getCollapsedEdge());
		}
	}
	((vector<Edge*>*)edgeList)->insert(edgeList->end(),newEdges->begin(),newEdges->end());
	delete newEdges;
}

bool bsgGreaterThan(BufferSubgraph *first,BufferSubgraph *second) {
	if (first->compareTo(second)>0)
		return true;
	else
		return false;
}

vector<BufferSubgraph*>* BufferOp::createSubgraphs(){
	vector<BufferSubgraph*> *subgraphList=new vector<BufferSubgraph*>();
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		if (!node->isVisited()) {
			BufferSubgraph *subgraph=new BufferSubgraph(cga);
			subgraph->create(node);
			subgraphList->push_back(subgraph);
		}
	}
	/**
	*Sort the subgraphs in descending order of their rightmost coordinate.
	*This ensures that when the Polygons for the subgraphs are built,
	*subgraphs for shells will have been built before the subgraphs for
	*any holes they contain.
	*/
	sort(subgraphList->begin(),subgraphList->end(),bsgGreaterThan);
	return subgraphList;
}

void BufferOp::buildSubgraphs(vector<BufferSubgraph*> *subgraphList,PolygonBuilder *polyBuilder){
	for(int i=0;i<(int)subgraphList->size();i++) {
		BufferSubgraph *subgraph=(*subgraphList)[i];
		Coordinate& p=subgraph->getRightmostCoordinate();
		int outsideDepth=0;
		if (polyBuilder->containsPoint(p))
			outsideDepth=1;
		subgraph->computeDepth(outsideDepth);
		subgraph->findResultEdges();
		polyBuilder->add(subgraph->getDirectedEdges(),subgraph->getNodes());
	}
}

Geometry* BufferOp::computeGeometry(vector<Polygon*> *resultPolyList){
	vector<Geometry*> *geomList=new vector<Geometry*>();
	for(int i=0;i<(int)resultPolyList->size();i++) {
		geomList->push_back((*resultPolyList)[i]);
	}
	Geometry *g=geomFact->buildGeometry(geomList);
	delete geomList;
	return g;
}

/**
*toLineStrings converts a list of Edges to LineStrings.
*/
Geometry* BufferOp::toLineStrings(EdgeList *edges){
	vector<Geometry*> *geomList=new vector<Geometry*>();
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		CoordinateList *pts=e->getCoordinates();
		LineString *line=geomFact->createLineString(pts);
		geomList->push_back(line);
	}
	Geometry *geom=geomFact->buildGeometry(geomList);
	return geom;
}
}

