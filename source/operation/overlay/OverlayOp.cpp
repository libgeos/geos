#include "opOverlay.h"
#include "stdio.h"
#include "util.h"

PointLocator* OverlayOp::ptLocator=new PointLocator();

Geometry* OverlayOp::overlayOp(Geometry *geom0,Geometry *geom1,int opCode) {
	OverlayOp *gov=new OverlayOp(geom0,geom1);
	Geometry* geomOv=gov->getResultGeometry(opCode);
	return geomOv;
}

bool OverlayOp::isResultOfOp(Label *label,int opCode) {
	int loc0=label->getLocation(0);
	int loc1=label->getLocation(1);
	return isResultOfOp(loc0,loc1,opCode);
}


/**
* This method will handle arguments of Location.NULL correctly
*
* @return true if the locations correspond to the opCode
*/
bool OverlayOp::isResultOfOp(int loc0,int loc1,int opCode) {
	if (loc0==Location::BOUNDARY) loc0=Location::INTERIOR;
	if (loc1==Location::BOUNDARY) loc1=Location::INTERIOR;
	switch (opCode) {
		case INTERSECTION:
			return loc0==Location::INTERIOR && loc1==Location::INTERIOR;
	case UNION:
		return loc0==Location::INTERIOR || loc1==Location::INTERIOR;
	case DIFFERENCE:
		return loc0==Location::INTERIOR && loc1!=Location::INTERIOR;
	case SYMDIFFERENCE:
		return (loc0==Location::INTERIOR && loc1!=Location::INTERIOR) 
			|| (loc0!=Location::INTERIOR && loc1==Location::INTERIOR);
	}
	return false;
}

OverlayOp::OverlayOp(Geometry *g0,Geometry *g1): GeometryGraphOperation(g0,g1) {
	graph=new PlanarGraph(new OverlayNodeFactory());
	geomFact=new GeometryFactory(g0->getPrecisionModel(),g0->getSRID());
	resultGeom=NULL;
	edgeList=new EdgeList();
	resultPolyList=new vector<Polygon*>();
	resultLineList=new vector<LineString*>();
	resultPointList=new vector<Point*>();
}

OverlayOp::~OverlayOp() {
	delete graph;
	delete geomFact;
	delete edgeList;
	delete resultPolyList;
	delete resultLineList;
	delete resultPointList;
}

Geometry* OverlayOp::getResultGeometry(int funcCode) {
	computeOverlay(funcCode);
	return resultGeom;
}

PlanarGraph* OverlayOp::getGraph() {
	return graph;
}

void OverlayOp::insertUniqueEdges(vector<Edge*> *edges) {
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		insertUniqueEdge(e);
	}
//for(int i=0;i<(int)edgeList->size();i++) {
//	Edge *e=(*edgeList)[i];
//	cout << endl << e->print() << endl;
//}
}

/**
* If edges which have undergone dimensional collapse are found,
* replace them with a new edge which is a L edge
*/
void OverlayOp::replaceCollapsedEdges() {
	vector<Edge*> *newEdges=new vector<Edge*>();
	vector<Edge*> *oldEdges=new vector<Edge*>();
	for(int i=0;i<(int)edgeList->size();i++) {
		Edge *e=(*edgeList)[i];
		if (e->isCollapsed()) {
			//Debug.print(e);
			newEdges->push_back(e->getCollapsedEdge());
		} else {
			//instead of removing from edgeList
			oldEdges->push_back(e);
		}
	}
	oldEdges->insert(oldEdges->end(),newEdges->begin(),newEdges->end());
	edgeList->assign(oldEdges->begin(),oldEdges->end());
}

/**
* Copy all nodes from an arg geometry into this graph.
* The node label in the arg geometry overrides any previously computed
* label for that argIndex.
* (E.g. a node may be an intersection node with
* a previously computed label of BOUNDARY,
* but in the original arg Geometry it is actually
* in the interior due to the Boundary Determination Rule)
*/
void OverlayOp::copyPoints(int argIndex) {
	map<Coordinate,Node*,CoordLT> *nodeMap=(*arg)[argIndex]->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *graphNode=it->second;
		Node *newNode=graph->addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,graphNode->getLabel()->getLocation(argIndex));
	}
}

/**
* Compute initial labelling for all DirectedEdges at each node.
* In this step, DirectedEdges will acquire a complete labelling
* (i.e. one with labels for both Geometries)
* only if they
* are incident on a node which has edges for both Geometries
*/
void OverlayOp::computeLabelling() {
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		node->getEdges()->computeLabelling(arg);
	}
	mergeSymLabels();
	updateNodeLabelling();
}

/**
* For nodes which have edges from only one Geometry incident on them,
* the previous step will have left their dirEdges with no labelling for the other
* Geometry.  However, the sym dirEdge may have a labelling for the other
* Geometry, so merge the two labels.
*/
void OverlayOp::mergeSymLabels() {
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		((DirectedEdgeStar*)node->getEdges())->mergeSymLabels();
		//node.print(System.out);
	}
}

void OverlayOp::updateNodeLabelling() {
	// update the labels for nodes
	// The label for a node is updated from the edges incident on it
	// (Note that a node may have already been labelled
	// because it is a point in one of the input geometries)
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		Label *lbl=((DirectedEdgeStar*)node->getEdges())->getLabel();
		node->getLabel()->merge(lbl);
	}
}

/**
* Incomplete nodes are nodes whose labels are incomplete.
* (e.g. the location for one Geometry is NULL).
* These are either isolated nodes,
* or nodes which have edges from only a single Geometry incident on them.
*
* Isolated nodes are found because nodes in one graph which don't intersect
* nodes in the other are not completely labelled by the initial process
* of adding nodes to the nodeList.
* To complete the labelling we need to check for nodes that lie in the
* interior of edges, and in the interior of areas.
* <p>
* When each node labelling is completed, the labelling of the incident
* edges is updated, to complete their labelling as well.
*/
void OverlayOp::labelIncompleteNodes() {
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *n=it->second;
		Label *label=n->getLabel();
		if (n->isIsolated()) {
			if (label->isNull(0))
				labelIncompleteNode(n,0);
			else
				labelIncompleteNode(n, 1);
		}
		// now update the labelling for the DirectedEdges incident on this node
		((DirectedEdgeStar*)n->getEdges())->updateLabelling(label);
		//n.print(System.out);
	}
}

/**
* Label an isolated node with its relationship to the target geometry.
*/
void OverlayOp::labelIncompleteNode(Node *n,int targetIndex) {
	int loc=ptLocator->locate(n->getCoordinate(),(*arg)[targetIndex]->getGeometry());
	n->getLabel()->setLocation(targetIndex,loc);
}

/**
* Find all edges whose label indicates that they are in the result area(s),
* according to the operation being performed.  Since we want polygon shells to be
* oriented CW, choose dirEdges with the interior of the result on the RHS.
* Mark them as being in the result.
* Interior Area edges are the result of dimensional collapses.
* They do not form part of the result area boundary.
*/
void OverlayOp::findResultAreaEdges(int opCode) {
	vector<EdgeEnd*> *ee=graph->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*) (*ee)[i];
		// mark all dirEdges with the appropriate label
		Label *label=de->getLabel();
		if (label->isArea()
			&& !de->isInteriorAreaEdge()
			&& isResultOfOp(label->getLocation(0,Position::RIGHT),
							label->getLocation(1,Position::RIGHT),
							opCode)
			) {
				de->setInResult(true);
				//Debug.print("in result "); Debug.println(de);
		}
	}
}

/**
* If both a dirEdge and its sym are marked as being in the result, cancel
* them out.
*/
void OverlayOp::cancelDuplicateResultEdges() {
	// remove any dirEdges whose sym is also included
	// (they "cancel each other out")
	vector<EdgeEnd*> *ee=graph->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*) (*ee)[i];
		DirectedEdge *sym=de->getSym();
		if (de->isInResult() && sym->isInResult()) {
			de->setInResult(false);
			sym->setInResult(false);
			//Debug.print("cancelled "); Debug.println(de); Debug.println(sym);
		}
	}
}

/**
* This method is used to decide if a point node should be included in the result or not.
*
* @return true if the coord point is covered by a result Line or Area geometry
*/
bool OverlayOp::isCoveredByLA(Coordinate& coord) {
	if (isCovered(coord,resultLineList)) return true;
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}
/**
* This method is used to decide if an L edge should be included in the result or not.
*
* @return true if the coord point is covered by a result Area geometry
*/
bool OverlayOp::isCoveredByA(Coordinate& coord) {
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}

/**
* @return true if the coord is located in the interior or boundary of
* a geometry in the list.
*/
bool OverlayOp::isCovered(Coordinate& coord,vector<Geometry*> *geomList) {
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/**
* @return true if the coord is located in the interior or boundary of
* a geometry in the list.
*/
bool OverlayOp::isCovered(Coordinate& coord,vector<LineString*> *geomList) {
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/**
* @return true if the coord is located in the interior or boundary of
* a geometry in the list.
*/
bool OverlayOp::isCovered(Coordinate& coord,vector<Polygon*> *geomList) {
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

Geometry* OverlayOp::computeGeometry(vector<Point*> *nResultPointList,
                              vector<LineString*> *nResultLineList,
                              vector<Polygon*> *nResultPolyList) {
	vector<Geometry*> *geomList=new vector<Geometry*>();
	// element geometries of the result are always in the order P,L,A
	geomList->insert(geomList->end(),nResultPointList->begin(),nResultPointList->end());
	geomList->insert(geomList->end(),nResultLineList->begin(),nResultLineList->end());
	geomList->insert(geomList->end(),nResultPolyList->begin(),nResultPolyList->end());
	// build the most specific geometry possible
	return geomFact->buildGeometry(geomList);
}

void OverlayOp::computeOverlay(int opCode) {
	// copy points from input Geometries.
	// This ensures that any Point geometries
	// in the input are considered for inclusion in the result set

	copyPoints(0);
	copyPoints(1);
	// node the input Geometries
	(*arg)[0]->computeSelfNodes(li);
	(*arg)[1]->computeSelfNodes(li);
	// compute intersections between edges of the two input geometries
	(*arg)[0]->computeEdgeIntersections((*arg)[1],li,true);
	
	vector<Edge*> *baseSplitEdges=new vector<Edge*>();
	(*arg)[0]->computeSplitEdges(baseSplitEdges);
	(*arg)[1]->computeSplitEdges(baseSplitEdges);

	vector<Edge*> *splitEdges=baseSplitEdges;
	/* NO LONGER USED
	// if we are working in fixed precision, we must renode to ensure noding is complete
	if (makePrecise) {
	List splitEdges1=completeEdgeNoding(baseSplitEdges);
	splitEdges=completeEdgeNoding(splitEdges1);
	}
	*/
	// add the noded edges to this result graph

	insertUniqueEdges(baseSplitEdges);
	computeLabelsFromDepths();
	replaceCollapsedEdges();
	//Debug.println(edgeList);
	graph->addEdges(edgeList);
	computeLabelling();
	//Debug.printWatch();
	labelIncompleteNodes();
	//Debug.printWatch();
	//nodeMap.print(System.out);

	/**
	* The ordering of building the result Geometries is important.
	* Areas must be built before lines, which must be built before points.
	* This is so that lines which are covered by areas are not included
	* explicitly, and similarly for points.
	*/
	findResultAreaEdges(opCode);
	cancelDuplicateResultEdges();
	PolygonBuilder *polyBuilder=new PolygonBuilder(geomFact,cga);
	polyBuilder->add(graph);
	resultPolyList=polyBuilder->getPolygons();
	LineBuilder *lineBuilder=new LineBuilder(this,geomFact,ptLocator);
	resultLineList=lineBuilder->build(opCode);
	PointBuilder *pointBuilder=new PointBuilder(this,geomFact,ptLocator);
	resultPointList=pointBuilder->build(opCode);
	// gather the results from all calculations into a single Geometry for the result set
	resultGeom=computeGeometry(resultPointList,resultLineList,resultPolyList);
}

/**
* Insert an edge from one of the noded input graphs.
* Checks edges that are inserted to see if an
* identical edge already exists.
* If so, the edge is not inserted, but its label is merged
* with the existing edge.
*/
void OverlayOp::insertUniqueEdge(Edge *e) {
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
			labelToMerge=new Label(*(e->getLabel()));
			labelToMerge->flip();
		}
		Depth *depth=existingEdge->getDepth();
		// if this is the first duplicate found for this edge, initialize the depths
		///*
		if (depth->isNull()) {
			depth->add(existingLabel);
		}
		//*/
		depth->add(labelToMerge);

		existingLabel->merge(labelToMerge);
		//Debug.print("inserted edge: "); Debug.println(e);
		//Debug.print("existing edge: "); Debug.println(existingEdge);
	} else {  // no matching existing edge was found
		// add this new edge to the list of edges in this graph
		//e.setName(name+edges.size());
		//e.getDepth().add(e.getLabel());
		edgeList->push_back(e);
	}
}

/**
* Update the labels for edges according to their depths.
* For each edge, the depths are first normalized.
* Then, if the depths for the edge are equal,
* this edge must have collapsed into a line edge.
* If the depths are not equal, update the label
* with the locations corresponding to the depths
* (i.e. a depth of 0 corresponds to a Location of EXTERIOR,
* a depth of 1 corresponds to INTERIOR)
*/
void OverlayOp::computeLabelsFromDepths() {
	for(int j=0;j<(int)edgeList->size();j++) {
		Edge *e=(*edgeList)[j];
		Label *lbl=e->getLabel();
		Depth *depth=e->getDepth();
		/**
		* Only check edges for which there were duplicates,
		* since these are the only ones which might
		* be the result of dimensional collapses.
		*/
		if (!depth->isNull()) {
			depth->normalize();
			for (int i=0;i<2;i++) {
				if (!lbl->isNull(i) && lbl->isArea() && !depth->isNull(i)) {
					/**
					* if the depths are equal, this edge is the result of
					* the dimensional collapse of two or more edges.
					* It has the same location on both sides of the edge,
					* so it has collapsed to a line.
					*/
					if (depth->getDelta(i)==0) {
						lbl->toLine(i);
					} else {
						/**
						* This edge may be the result of a dimensional collapse,
						* but it still has different locations on both sides.  The
						* label of the edge must be updated to reflect the resultant
						* side locations indicated by the depth values.
						*/
						Assert::isTrue(!depth->isNull(i,Position::LEFT),"depth of LEFT side has not been initialized");
						lbl->setLocation(i,Position::LEFT,depth->getLocation(i,Position::LEFT));
						Assert::isTrue(!depth->isNull(i,Position::RIGHT),"depth of RIGHT side has not been initialized");
						lbl->setLocation(i,Position::RIGHT,depth->getLocation(i,Position::RIGHT));
					}
				}
			}
		}
	}
}
