/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opOverlay.h>
#include <stdio.h>
#include <geos/util.h>

#define DEBUG 0
#define COMPUTE_Z 1
#define USE_ELEVATION_MATRIX 1
#define USE_INPUT_AVGZ 0

namespace geos {

Geometry*
OverlayOp::overlayOp(const Geometry *geom0,const Geometry *geom1,int opCode)
	// throw(TopologyException *)
{
	OverlayOp gov(geom0, geom1);
	return gov.getResultGeometry(opCode);
}

bool
OverlayOp::isResultOfOp(Label *label,int opCode)
{
	int loc0=label->getLocation(0);
	int loc1=label->getLocation(1);
	return isResultOfOp(loc0,loc1,opCode);
}


/*
 * This method will handle arguments of Location.NULL correctly
 *
 * @return true if the locations correspond to the opCode
 */
bool
OverlayOp::isResultOfOp(int loc0,int loc1,int opCode)
{
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

OverlayOp::OverlayOp(const Geometry *g0, const Geometry *g1): GeometryGraphOperation(g0,g1)
{
	graph=new PlanarGraph(new OverlayNodeFactory());

	/*
	 * Use factory of primary geometry.
	 * Note that this does NOT handle mixed-precision arguments
	 * where the second arg has greater precision than the first.
	 */
	geomFact=g0->getFactory();
	resultGeom=NULL;
	edgeList=new EdgeList();
	resultPolyList=NULL;
	resultLineList=NULL;
	resultPointList=NULL;
	ptLocator=new PointLocator();
#if COMPUTE_Z
#if USE_INPUT_AVGZ
	avgz[0] = DoubleNotANumber;
	avgz[1] = DoubleNotANumber;
	avgzcomputed[0] = false;
	avgzcomputed[1] = false;
#endif // USE_INPUT_AVGZ

	Envelope env(*(g0->getEnvelopeInternal()));
	env.expandToInclude(g1->getEnvelopeInternal());
#if USE_ELEVATION_MATRIX
	elevationMatrix = new ElevationMatrix(env, 3, 3);
	elevationMatrix->add(g0);
	elevationMatrix->add(g1);
#if DEBUG
	cerr<<elevationMatrix->print()<<endl;
#endif
#endif // USE_ELEVATION_MATRIX
#endif // COMPUTE_Z
}

OverlayOp::~OverlayOp()
{
	delete graph;
	delete edgeList;
	delete resultPolyList;
	delete resultLineList;
	delete resultPointList;
	delete ptLocator;
	for (unsigned int i=0; i<dupEdges.size(); i++)
		delete dupEdges[i];
#if USE_ELEVATION_MATRIX
	delete elevationMatrix;
#endif
}

Geometry*
OverlayOp::getResultGeometry(int funcCode)
	//throw(TopologyException *)
{
	computeOverlay(funcCode); // this can throw TopologyException *
	return resultGeom;
}

PlanarGraph*
OverlayOp::getGraph()
{
	return graph;
}

void
OverlayOp::insertUniqueEdges(vector<Edge*> *edges)
{
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		insertUniqueEdge(e);
	}

#if DEBUG
	cerr<<"OverlayOp::insertUniqueEdges("<<edges->size()<<"): "<<endl;
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		if ( ! e ) cerr <<" NULL"<<endl;
		cerr <<" "<< e->print() << endl;
	}
#endif // DEBUG

}

/*
 * If edges which have undergone dimensional collapse are found,
 * replace them with a new edge which is a L edge
 */
void
OverlayOp::replaceCollapsedEdges()
{
	vector<Edge*> *newEdges=new vector<Edge*>();
	vector<Edge*> *oldEdges=new vector<Edge*>();
	for(int i=0;i<(int)edgeList->getEdges()->size();i++) {
		Edge *e=edgeList->get(i);
		if (e->isCollapsed()) {
			//Debug.print(e);
			newEdges->push_back(e->getCollapsedEdge());
			delete e;
		} else {
			//instead of removing from edgeList
			oldEdges->push_back(e);
		}
	}
	oldEdges->insert(oldEdges->end(),newEdges->begin(),newEdges->end());
	edgeList->getEdges()->assign(oldEdges->begin(),oldEdges->end());
	delete oldEdges;
	delete newEdges;
}

/*
 * Copy all nodes from an arg geometry into this graph.
 * The node label in the arg geometry overrides any previously computed
 * label for that argIndex.
 * (E.g. a node may be an intersection node with
 * a previously computed label of BOUNDARY,
 * but in the original arg Geometry it is actually
 * in the interior due to the Boundary Determination Rule)
 */
void
OverlayOp::copyPoints(int argIndex)
{
	map<Coordinate,Node*,CoordLT> *nodeMap=(*arg)[argIndex]->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *graphNode=it->second;
		Node *newNode=graph->addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,graphNode->getLabel()->getLocation(argIndex));
	}
}

/*
 * Compute initial labelling for all DirectedEdges at each node.
 * In this step, DirectedEdges will acquire a complete labelling
 * (i.e. one with labels for both Geometries)
 * only if they
 * are incident on a node which has edges for both Geometries
 */
void
OverlayOp::computeLabelling()
	//throw(TopologyException *) // and what else ?
{
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;

#if DEBUG
	cerr<<"OverlayOp::computeLabelling(): at call time: "<<edgeList->print()<<endl;
#endif

#if DEBUG
	cerr<<"OverlayOp::computeLabelling() scanning "<<nodeMap->size()<<" nodes from map:"<<endl;
#endif

	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
#if DEBUG
		cerr<<"     "<<node->print()<<" has "<<node->getEdges()->getEdges()->size()<<" edgeEnds"<<endl;
#endif
		node->getEdges()->computeLabelling(arg);
	}
#if DEBUG
	cerr<<"OverlayOp::computeLabelling(): after edge labelling: "<<edgeList->print()<<endl;
#endif
	mergeSymLabels();
#if DEBUG
	cerr<<"OverlayOp::computeLabelling(): after labels sym merging: "<<edgeList->print()<<endl;
#endif
	updateNodeLabelling();
#if DEBUG
	cerr<<"OverlayOp::computeLabelling(): after node labeling update: "<<edgeList->print()<<endl;
#endif
}

/*
 * For nodes which have edges from only one Geometry incident on them,
 * the previous step will have left their dirEdges with no labelling
 * for the other Geometry. 
 * However, the sym dirEdge may have a labelling for the other
 * Geometry, so merge the two labels.
 */
void
OverlayOp::mergeSymLabels()
{
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;

#if DEBUG
	cerr<<"OverlayOp::mergeSymLabels() scanning "<<nodeMap->size()<<" nodes from map:"<<endl;
#endif

	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		((DirectedEdgeStar*)node->getEdges())->mergeSymLabels();
#if DEBUG
		cerr<<"     "<<node->print()<<endl;
#endif
		//node.print(System.out);
	}
}

void
OverlayOp::updateNodeLabelling()
{
	// update the labels for nodes
	// The label for a node is updated from the edges incident on it
	// (Note that a node may have already been labelled
	// because it is a point in one of the input geometries)
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
#if DEBUG
	cerr<<"OverlayOp::updateNodeLabelling() scanning "<<nodeMap->size()<<" nodes from map:"<<endl;
#endif
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		Label *lbl=((DirectedEdgeStar*)node->getEdges())->getLabel();
		node->getLabel()->merge(lbl);
#if DEBUG
		cerr<<"     "<<node->print()<<endl;
#endif
	}
}

/*
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
 *
 * When each node labelling is completed, the labelling of the incident
 * edges is updated, to complete their labelling as well.
 */
void
OverlayOp::labelIncompleteNodes()
{
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
#if DEBUG
	cerr<<"OverlayOp::labelIncompleteNodes() scanning "<<nodeMap->size()<<" nodes from map:"<<endl;
#endif
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

/*
 * Label an isolated node with its relationship to the target geometry.
 */
void
OverlayOp::labelIncompleteNode(Node *n, int targetIndex)
{
#if DEBUG
	cerr<<"OverlayOp::labelIncompleteNode("<<n->print()<<", "<<targetIndex<<")"<<endl;
#endif
	const Geometry *targetGeom = (*arg)[targetIndex]->getGeometry();
	int loc=ptLocator->locate(n->getCoordinate(), targetGeom);
	n->getLabel()->setLocation(targetIndex,loc);

#if DEBUG
	cerr<<"   after location set: "<<n->print()<<endl;
#endif

#if COMPUTE_Z
	/*
	 * If this node has been labeled INTERIOR of a line
	 * or BOUNDARY of a polygon we must merge
	 * Z values of the intersected segment.
	 * The intersection point has been already computed
	 * by LineIntersector invoked by CGAlgorithms::isOnLine
	 * invoked by PointLocator.
	 */
	const LineString *line = dynamic_cast<const LineString *>(targetGeom);
	if ( loc == Location::INTERIOR && line )
	{
		mergeZ(n, line);
	}
	const Polygon *poly = dynamic_cast<const Polygon *>(targetGeom);
	if ( loc == Location::BOUNDARY && poly )
	{
		mergeZ(n, poly);
	}
#if USE_INPUT_AVGZ
	if ( loc == Location::INTERIOR && poly )
	{
		n->addZ(getAverageZ(targetIndex));
	}
#endif // USE_INPUT_AVGZ
#endif // COMPUTE_Z
}

double
OverlayOp::getAverageZ(const Polygon *poly)
{
	double totz = 0.0;
	int zcount = 0;

	const CoordinateSequence *pts =
		poly->getExteriorRing()->getCoordinatesRO();
	for (int i=0; i<pts->getSize(); i++)
	{
		const Coordinate &c = pts->getAt(i);
		if ( !ISNAN(c.z) )
		{
			totz += c.z;
			zcount++;
		}
	}

	if ( zcount ) return totz/zcount;
	else return DoubleNotANumber;
}

/*
 * This caches result to avoid multiple scans
 */
double
OverlayOp::getAverageZ(int targetIndex)
{
	if ( avgzcomputed[targetIndex] ) return avgz[targetIndex];

	const Geometry *targetGeom = (*arg)[targetIndex]->getGeometry();

	Assert::isTrue(targetGeom->getGeometryTypeId() == GEOS_POLYGON, 
		"OverlayOp::getAverageZ(int) called with a ! polygon");

	avgz[targetIndex] = getAverageZ((const Polygon *)targetGeom);
	avgzcomputed[targetIndex] = true;
	return avgz[targetIndex];
}

/*
 * Merge Z values of node with those of the segment or vertex in
 * the given Polygon it is on.
 */
int
OverlayOp::mergeZ(Node *n, const Polygon *poly) const
{
	const LineString *ls;
	int found = 0;
	ls = (const LineString *)poly->getExteriorRing();
	found = mergeZ(n, ls);
	if ( found ) return 1;
	for (int i=0; i<poly->getNumInteriorRing(); i++)
	{
		ls = (const LineString *)poly->getInteriorRingN(i);
		found = mergeZ(n, ls);
		if ( found ) return 1;
	}
	return 0;
}

/*
 * Merge Z values of node with those of the segment or vertex in
 * the given LineString it is on.
 * @returns 1 if an intersection is found, 0 otherwise.
 */
int
OverlayOp::mergeZ(Node *n, const LineString *line) const
{
	const CoordinateSequence *pts = line->getCoordinatesRO();
	const Coordinate &p = n->getCoordinate();
	RobustLineIntersector li;
	for(int i=1;i<pts->getSize();i++) {
		Coordinate p0=pts->getAt(i-1);
		Coordinate p1=pts->getAt(i);	
		li.computeIntersection(p, p0, p1);
		if (li.hasIntersection()) {
			if ( p == p0 ) n->addZ(p0.z);
			else if ( p == p1 ) n->addZ(p1.z);
			else {
				//n->addZ(p0.z);
				//n->addZ(p1.z);
				n->addZ(LineIntersector::interpolateZ(p,
					p0, p1));
			}
			return 1;
		}
	}
	return 0;
}

/*
 * Find all edges whose label indicates that they are in the result area(s),
 * according to the operation being performed.  Since we want polygon shells
 * to be oriented CW, choose dirEdges with the interior of the result on the
 * RHS.
 * Mark them as being in the result.
 * Interior Area edges are the result of dimensional collapses.
 * They do not form part of the result area boundary.
 */
void
OverlayOp::findResultAreaEdges(int opCode)
{
	vector<EdgeEnd*> *ee=graph->getEdgeEnds();
	for(unsigned int i=0;i<ee->size();i++) {
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

/*
 * If both a dirEdge and its sym are marked as being in the result, cancel
 * them out.
 */
void
OverlayOp::cancelDuplicateResultEdges()
{
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

/*
 * This method is used to decide if a point node should be included
 * in the result or not.
 *
 * @return true if the coord point is covered by a result Line or
 *	Area geometry
 */
bool
OverlayOp::isCoveredByLA(const Coordinate& coord)
{
	if (isCovered(coord,resultLineList)) return true;
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}

/*
 * This method is used to decide if an L edge should be included
 * in the result or not.
 *
 * @return true if the coord point is covered by a result Area geometry
 */
bool
OverlayOp::isCoveredByA(const Coordinate& coord)
{
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}

/*
 * @return true if the coord is located in the interior or boundary of
 * a geometry in the list.
 */
bool
OverlayOp::isCovered(const Coordinate& coord,vector<Geometry*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*
 * @return true if the coord is located in the interior or boundary of
 * a geometry in the list.
 */
bool
OverlayOp::isCovered(const Coordinate& coord,vector<LineString*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*
 * @return true if the coord is located in the interior or boundary of
 * a geometry in the list.
 */
bool
OverlayOp::isCovered(const Coordinate& coord,vector<Polygon*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator->locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*
 * Build a Geometry containing given elements
 * This function will take ownership of vectors elements.
 */
Geometry*
OverlayOp::computeGeometry(vector<Point*> *nResultPointList,
                              vector<LineString*> *nResultLineList,
                              vector<Polygon*> *nResultPolyList) {
	int i;
	vector<Geometry*> *geomList=new vector<Geometry*>();
	// element geometries of the result are always in the order P,L,A
	for(i=0;i<(int)nResultPointList->size();i++) {
		Point *pt=(*nResultPointList)[i];
		geomList->push_back(pt);
	}
	for(i=0;i<(int)nResultLineList->size();i++) {
		LineString *ls=(*nResultLineList)[i];
		geomList->push_back(ls);
	}
	for(i=0;i<(int)nResultPolyList->size();i++) {
		Polygon *q=(*nResultPolyList)[i];
		geomList->push_back(q);
	}
	// build the most specific geometry possible
	Geometry *g=geomFact->buildGeometry(geomList);
	return g;
}

void
OverlayOp::computeOverlay(int opCode)
	//throw(TopologyException *)
{
	vector<Edge*> *baseSplitEdges=NULL;
	PolygonBuilder *polyBuilder=NULL;
	LineBuilder *lineBuilder=NULL;
	PointBuilder *pointBuilder=NULL;

	// copy points from input Geometries.
	// This ensures that any Point geometries
	// in the input are considered for inclusion in the result set

	copyPoints(0);
	copyPoints(1);


	// node the input Geometries
	delete (*arg)[0]->computeSelfNodes(li,false);
	delete (*arg)[1]->computeSelfNodes(li,false);

#if DEBUG
	cerr<<"OverlayOp::computeOverlay: computed SelfNodes"<<endl;
#endif

	// compute intersections between edges of the two input geometries
	delete (*arg)[0]->computeEdgeIntersections((*arg)[1],li,true);

#if DEBUG
	cerr<<"OverlayOp::computeOverlay: computed EdgeIntersections"<<endl;
	cerr<<"OverlayOp::computeOverlay: li: "<<li->toString()<<endl;
#endif


	baseSplitEdges = new vector<Edge*>();
	(*arg)[0]->computeSplitEdges(baseSplitEdges);
	(*arg)[1]->computeSplitEdges(baseSplitEdges);

	// add the noded edges to this result graph
	insertUniqueEdges(baseSplitEdges);
	computeLabelsFromDepths();
	replaceCollapsedEdges();
	//Debug.println(edgeList);

    	// debugging only
    	//NodingValidator nv = new NodingValidator(edgeList.getEdges());
    	//nv.checkValid();

	graph->addEdges(edgeList->getEdges());

	try {
		// this can throw TopologyException *
		computeLabelling();

		//Debug.printWatch();
		labelIncompleteNodes();
		//Debug.printWatch();
		//nodeMap.print(System.out);


		/*
		 * The ordering of building the result Geometries is important.
		 * Areas must be built before lines, which must be built
		 * before points.
		 * This is so that lines which are covered by areas are not
		 * included explicitly, and similarly for points.
		 */
		findResultAreaEdges(opCode);
		cancelDuplicateResultEdges();

		polyBuilder=new PolygonBuilder(geomFact,cga);
		
		// might throw a TopologyException *
		polyBuilder->add(graph);

		vector<Geometry*> *gv=polyBuilder->getPolygons();
		resultPolyList=new vector<Polygon*>();
		for(int i=0;i<(int)gv->size();i++) {
			resultPolyList->push_back((Polygon*)(*gv)[i]);
		}
		delete gv;
		lineBuilder=new LineBuilder(this,geomFact,ptLocator);
		resultLineList=lineBuilder->build(opCode);
		pointBuilder=new PointBuilder(this,geomFact,ptLocator);
		resultPointList=pointBuilder->build(opCode);
		// gather the results from all calculations into a single
		// Geometry for the result set
		resultGeom=computeGeometry(resultPointList,resultLineList,resultPolyList);
#if USE_ELEVATION_MATRIX
		elevationMatrix->elevate(resultGeom);
#endif // USE_ELEVATION_MATRIX
		


	} catch (...) {
		delete baseSplitEdges;
		delete polyBuilder;
		delete lineBuilder;
		delete pointBuilder;
		throw;
	}


	delete polyBuilder;
	delete lineBuilder;
	delete pointBuilder;
	delete baseSplitEdges;
}

/*
 * Insert an edge from one of the noded input graphs.
 * Checks edges that are inserted to see if an
 * identical edge already exists.
 * If so, the edge is not inserted, but its label is merged
 * with the existing edge.
 */
void
OverlayOp::insertUniqueEdge(Edge *e)
{
	//Debug.println(e);
#if DEBUG
	cerr<<"OverlayOp::insertUniqueEdge("<<e->print()<<")"<<endl;
#endif

	int foundIndex=edgeList->findEdgeIndex(e);
	// If an identical edge already exists, simply update its label
	if (foundIndex>=0) {
#if DEBUG
		cerr<<"  found identical edge, should merge Z"<<endl;
#endif
		Edge *existingEdge=edgeList->get(foundIndex);
		Label *existingLabel=existingEdge->getLabel();
		Label *labelToMerge=e->getLabel();

		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (!existingEdge->isPointwiseEqual(e)) {
//			labelToMerge=new Label(e->getLabel());
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
		dupEdges.push_back(e);
	} else {  // no matching existing edge was found
#if DEBUG
		cerr<<"  no matching existing edge"<<endl;
#endif
		// add this new edge to the list of edges in this graph
		//e.setName(name+edges.size());
		//e.getDepth().add(e.getLabel());
		edgeList->add(e);
	}
}

/*
 * Update the labels for edges according to their depths.
 * For each edge, the depths are first normalized.
 * Then, if the depths for the edge are equal,
 * this edge must have collapsed into a line edge.
 * If the depths are not equal, update the label
 * with the locations corresponding to the depths
 * (i.e. a depth of 0 corresponds to a Location of EXTERIOR,
 * a depth of 1 corresponds to INTERIOR)
 */
void
OverlayOp::computeLabelsFromDepths()
{
	for(int j=0;j<(int)edgeList->getEdges()->size();j++) {
		Edge *e=edgeList->get(j);
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.37  2004/12/08 14:31:17  strk
 * elevationMatrix deleted by destructor
 *
 * Revision 1.36  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.35  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.34  2004/11/26 09:22:50  strk
 * Added FINITE(x) macro and its use.
 * Made input geoms average Z computation optional in OverlayOp.
 *
 * Revision 1.33  2004/11/24 18:10:42  strk
 * Stricter handling of USE_ELEVATION_MATRIX define
 *
 * Revision 1.32  2004/11/23 19:53:07  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.31  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.30  2004/11/22 15:51:52  strk
 * Added interpolation of containing geometry's average Z for point_in_poly case.
 *
 * Revision 1.29  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.28  2004/11/20 17:16:10  strk
 * Handled Z merging for point on polygon boundary case.
 *
 * Revision 1.27  2004/11/20 16:25:17  strk
 * Added Z computation for point on line case.
 *
 * Revision 1.26  2004/11/17 15:09:08  strk
 * Changed COMPUTE_Z defaults to be more conservative
 *
 * Revision 1.25  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.24  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.23  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.22  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.21  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.20  2004/06/15 20:13:42  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.19  2004/05/17 08:34:31  strk
 * reduced stack allocations, try/catch blocks in ::overlayOp
 *
 * Revision 1.18  2004/05/03 10:43:43  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.17  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.16  2004/04/14 13:14:29  strk
 * Removed deletion of externally pointed GeometryFactory from OverlayOp destructor
 *
 * Revision 1.15  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.14  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.13  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.11  2003/11/12 16:14:56  strk
 * Added some more throw specifications and cleanup on exception (leaks removed).
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

