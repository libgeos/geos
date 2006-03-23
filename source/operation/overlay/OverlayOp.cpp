/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/OverlayOp.java rev. 1.23
 *
 **********************************************************************/

#include <cassert>
#include <functional>
#include <vector>

#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/ElevationMatrix.h>
#include <geos/operation/overlay/OverlayNodeFactory.h>
#include <geos/operation/overlay/PolygonBuilder.h>
#include <geos/operation/overlay/LineBuilder.h>
#include <geos/operation/overlay/PointBuilder.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/PrecisionModel.h>

#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geomgraph/EdgeEndStar.h>
#include <geos/geomgraph/DirectedEdgeStar.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/Position.h>

#include <geos/geomgraph/index/SegmentIntersector.h>

#include <geos/util/TopologyException.h>

#include <geos/precision/SimpleGeometryPrecisionReducer.h>

#include <memory> // for auto_ptr

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#define COMPUTE_Z 1
#define USE_ELEVATION_MATRIX 1
#define USE_INPUT_AVGZ 0

// Define this to have OverlayOp::overlayOp
// automatically try with reduced precisions
// on topology exceptions
//
#define TRY_REDUCED_GEOMS 1

using namespace std;
using namespace geos::geom;
using namespace geos::geomgraph;
using namespace geos::algorithm;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

// module-statics
namespace {

/**
 * If an exception it catched during result computation, try reducing precision
 *
 * So far I haven't seen any case in which more then a single iteration 
 * is required to make things work (but I haven't many tests).
 *
 * Possible problems:
 *	- If the input is invalid, this could be a very costly operation,
 * 	  as precision reduction will hardly fix the invalidity.
 *
 *
 * Possible optimization include:
 *	- only reducing one geometry each iteration (swapping reduced geom)
 *	- decrementing the precision by more then 1 unit each iteration
 */
Geometry*
reducedOverlayOp(const Geometry* g0, const Geometry* g1, int opCode)
{
	int maxPrecision=25;
	Geometry* ret=NULL;

	for (int precision=maxPrecision; precision; --precision)
	{
		auto_ptr<PrecisionModel> pm(new PrecisionModel(precision));
		cerr << "Trying with precision " << precision << endl;

		precision::SimpleGeometryPrecisionReducer reducer(pm.get());
		auto_ptr<Geometry> redGeom0(reducer.reduce(g0));
		auto_ptr<Geometry> redGeom1(reducer.reduce(g1));

		OverlayOp gov(redGeom0.get(), redGeom1.get());

		try {
			ret=gov.getResultGeometry(opCode);
		} catch (const util::TopologyException& ex) {
			if ( precision == 1 ) throw ex;
			//cerr << "Reduced precision (" << precision << "): " << ex.what() << endl;
		}

		if ( ret ) return ret;
	}

	return ret;
}

} // unnamed (module-statics)

/* static public */
Geometry*
OverlayOp::overlayOp(const Geometry *geom0, const Geometry *geom1, int opCode)
	// throw(TopologyException *)
{
	OverlayOp gov(geom0, geom1);
#ifdef TRY_REDUCED_GEOMS
	try
	{
#endif
		return gov.getResultGeometry(opCode);
#ifdef TRY_REDUCED_GEOMS
	}
	catch (const util::TopologyException& ex)
	{
		//cerr << "Original precision: " << ex.what() << endl;
		if ( gov.resultPrecisionModel->getType() == PrecisionModel::FIXED &&
			gov.resultPrecisionModel->getScale() == 1 )
		{
			// Already the smallest scale :(
			throw ex;
		}
		return reducedOverlayOp(geom0, geom1, opCode);
	}
#endif
}

/* static public */
bool
OverlayOp::isResultOfOp(Label *label,int opCode)
{
	int loc0=label->getLocation(0);
	int loc1=label->getLocation(1);
	return isResultOfOp(loc0,loc1,opCode);
}


/* static public */
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

OverlayOp::OverlayOp(const Geometry *g0, const Geometry *g1)

	:

	// this builds graphs in arg[0] and arg[1]
	GeometryGraphOperation(g0, g1),

	/*
	 * Use factory of primary geometry.
	 * Note that this does NOT handle mixed-precision arguments
	 * where the second arg has greater precision than the first.
	 */
	geomFact(g0->getFactory()),

	resultGeom(NULL),
	graph(OverlayNodeFactory::instance()),
	resultPolyList(NULL),
	resultLineList(NULL),
	resultPointList(NULL)

{

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
#if GEOS_DEBUG
	cerr<<elevationMatrix->print()<<endl;
#endif
#endif // USE_ELEVATION_MATRIX
#endif // COMPUTE_Z
}

OverlayOp::~OverlayOp()
{
	//delete edgeList;
	delete resultPolyList;
	delete resultLineList;
	delete resultPointList;
	for (unsigned int i=0; i<dupEdges.size(); i++)
		delete dupEdges[i];
#if USE_ELEVATION_MATRIX
	delete elevationMatrix;
#endif
}

#undef REDUCED_PRECISION_OVERLAY

#ifdef REDUCED_PRECISION_OVERLAY

/*public*/
Geometry*
OverlayOp::getResultGeometry(int funcCode)
	//throw(TopologyException *)
{
	int attempt=3;
	PrecisionModel pm;

	while (!resultGeom && attempt--)
	{
		try {
			computeOverlay(funcCode, &pm); // this can throw TopologyException *
		}
		catch (const util::TopologyException& ex)
		{
			cerr<<"Attempt "<<attempt+1<<" exception "<<ex.what()<<endl;
		}
	}
	return resultGeom;
}

/*private*/
Geometry*
OverlayOp::getResultGeometry(int funcCode, const PrecisionModel* pm)
{
	if ( pm ) setComputationPrecision(pm);
	computeOverlay(funcCode); // this can throw TopologyException *
	return resultGeom;
}

#else  // ndef REDUCED_PRECISION_OVERLAY

/*public*/
Geometry*
OverlayOp::getResultGeometry(int funcCode)
	//throw(TopologyException *)
{
	computeOverlay(funcCode);
	return resultGeom;
}

#endif // ndef REDUCED_PRECISION_OVERLAY

/*private*/
void
OverlayOp::insertUniqueEdges(vector<Edge*> *edges)
{
	for_each(edges->begin(), edges->end(),
			bind1st(mem_fun(&OverlayOp::insertUniqueEdge), this));

#if 0
	for(unsigned int i=0; i<edges->size(); ++i) {
		Edge *e=(*edges)[i];
		insertUniqueEdge(e);
	}
#endif

#if GEOS_DEBUG
	cerr<<"OverlayOp::insertUniqueEdges("<<edges->size()<<"): "<<endl;
	for(unsigned int i=0;i<edges->size();i++) {
		Edge *e=(*edges)[i];
		if ( ! e ) cerr <<" NULL"<<endl;
		cerr <<" "<< e->print() << endl;
	}
#endif // GEOS_DEBUG

}

/*private*/
void
OverlayOp::replaceCollapsedEdges()
{
	vector<Edge*> &edges=edgeList.getEdges();

	for(unsigned int i=0, nedges=edges.size(); i<nedges; ++i)
	{
		Edge *e=edges[i];
		if (e->isCollapsed()) {
			//Debug.print(e);
			edges[i]=e->getCollapsedEdge();
			delete e;
		} 
	}
}

/*private*/
void
OverlayOp::copyPoints(int argIndex)
{
	map<Coordinate*,Node*,CoordinateLessThen>&nodeMap=arg[argIndex]->getNodeMap()->nodeMap;
	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *graphNode=it->second;
		Node *newNode=graph.addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,graphNode->getLabel()->getLocation(argIndex));
	}
}

/*private*/
void
OverlayOp::computeLabelling()
	//throw(TopologyException *) // and what else ?
{
	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap=graph.getNodeMap()->nodeMap;

#if GEOS_DEBUG
	cerr<<"OverlayOp::computeLabelling(): at call time: "<<edgeList.print()<<endl;
#endif

#if GEOS_DEBUG
	cerr<<"OverlayOp::computeLabelling() scanning "<<nodeMap.size()<<" nodes from map:"<<endl;
#endif

	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *node=it->second;
#if GEOS_DEBUG
		cerr<<"     "<<node->print()<<" has "<<node->getEdges()->getEdges().size()<<" edgeEnds"<<endl;
#endif
		node->getEdges()->computeLabelling(&arg);
	}
#if GEOS_DEBUG
	cerr<<"OverlayOp::computeLabelling(): after edge labelling: "<<edgeList.print()<<endl;
#endif
	mergeSymLabels();
#if GEOS_DEBUG
	cerr<<"OverlayOp::computeLabelling(): after labels sym merging: "<<edgeList.print()<<endl;
#endif
	updateNodeLabelling();
#if GEOS_DEBUG
	cerr<<"OverlayOp::computeLabelling(): after node labeling update: "<<edgeList.print()<<endl;
#endif
}

/*private*/
void
OverlayOp::mergeSymLabels()
{
	map<Coordinate*,Node*,CoordinateLessThen>&nodeMap=graph.getNodeMap()->nodeMap;

#if GEOS_DEBUG
	cerr<<"OverlayOp::mergeSymLabels() scanning "<<nodeMap.size()<<" nodes from map:"<<endl;
#endif

	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *node=it->second;
		EdgeEndStar* ees=node->getEdges();
		assert(dynamic_cast<DirectedEdgeStar*>(ees));
		static_cast<DirectedEdgeStar*>(ees)->mergeSymLabels();
		//((DirectedEdgeStar*)node->getEdges())->mergeSymLabels();
#if GEOS_DEBUG
		cerr<<"     "<<node->print()<<endl;
#endif
		//node.print(System.out);
	}
}

/*private*/
void
OverlayOp::updateNodeLabelling()
{
	// update the labels for nodes
	// The label for a node is updated from the edges incident on it
	// (Note that a node may have already been labelled
	// because it is a point in one of the input geometries)
	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap=graph.getNodeMap()->nodeMap;
#if GEOS_DEBUG
	cerr<<"OverlayOp::updateNodeLabelling() scanning "<<nodeMap.size()<<" nodes from map:"<<endl;
#endif
	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *node=it->second;
		Label &lbl=((DirectedEdgeStar*)node->getEdges())->getLabel();
		node->getLabel()->merge(lbl);
#if GEOS_DEBUG
		cerr<<"     "<<node->print()<<endl;
#endif
	}
}

/*private*/
void
OverlayOp::labelIncompleteNodes()
{
	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap=graph.getNodeMap()->nodeMap;
#if GEOS_DEBUG
	cerr<<"OverlayOp::labelIncompleteNodes() scanning "<<nodeMap.size()<<" nodes from map:"<<endl;
#endif
	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
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

/*private*/
void
OverlayOp::labelIncompleteNode(Node *n, int targetIndex)
{
#if GEOS_DEBUG
	cerr<<"OverlayOp::labelIncompleteNode("<<n->print()<<", "<<targetIndex<<")"<<endl;
#endif
	const Geometry *targetGeom = arg[targetIndex]->getGeometry();
	int loc=ptLocator.locate(n->getCoordinate(), targetGeom);
	n->getLabel()->setLocation(targetIndex,loc);

#if GEOS_DEBUG
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

/*static private*/
double
OverlayOp::getAverageZ(const Polygon *poly)
{
	double totz = 0.0;
	int zcount = 0;

	const CoordinateSequence *pts =
		poly->getExteriorRing()->getCoordinatesRO();
	unsigned int npts=pts->getSize();
	for (unsigned int i=0; i<npts; ++i)
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

/*private*/
double
OverlayOp::getAverageZ(int targetIndex)
{
	if ( avgzcomputed[targetIndex] ) return avgz[targetIndex];

	const Geometry *targetGeom = arg[targetIndex]->getGeometry();

	// OverlayOp::getAverageZ(int) called with a ! polygon
	assert(targetGeom->getGeometryTypeId() == GEOS_POLYGON);

	avgz[targetIndex] = getAverageZ((const Polygon *)targetGeom);
	avgzcomputed[targetIndex] = true;
	return avgz[targetIndex];
}

/*private*/
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

/*private*/
int
OverlayOp::mergeZ(Node *n, const LineString *line) const
{
	const CoordinateSequence *pts = line->getCoordinatesRO();
	const Coordinate &p = n->getCoordinate();
	LineIntersector li;
	unsigned int size = pts->getSize();
	for(unsigned int i=1; i<size; ++i) {
		const Coordinate &p0=pts->getAt(i-1);
		const Coordinate &p1=pts->getAt(i);	
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

/*private*/
void
OverlayOp::findResultAreaEdges(int opCode)
{
	vector<EdgeEnd*> *ee=graph.getEdgeEnds();
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

/*private*/
void
OverlayOp::cancelDuplicateResultEdges()
{
	// remove any dirEdges whose sym is also included
	// (they "cancel each other out")
	vector<EdgeEnd*> *ee=graph.getEdgeEnds();
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

/*public*/
bool
OverlayOp::isCoveredByLA(const Coordinate& coord)
{
	if (isCovered(coord,resultLineList)) return true;
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}

/*public*/
bool
OverlayOp::isCoveredByA(const Coordinate& coord)
{
	if (isCovered(coord,resultPolyList)) return true;
	return false;
}

/*private*/
bool
OverlayOp::isCovered(const Coordinate& coord,vector<Geometry*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(*geomList)[i];
		int loc=ptLocator.locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*private*/
bool
OverlayOp::isCovered(const Coordinate& coord,vector<LineString*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator.locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*private*/
bool
OverlayOp::isCovered(const Coordinate& coord,vector<Polygon*> *geomList)
{
	for(int i=0;i<(int)geomList->size();i++) {
		Geometry *geom=(Geometry*)(*geomList)[i];
		int loc=ptLocator.locate(coord,geom);
		if (loc!=Location::EXTERIOR) return true;
	}
	return false;
}

/*private*/
Geometry*
OverlayOp::computeGeometry(vector<Point*> *nResultPointList,
                              vector<LineString*> *nResultLineList,
                              vector<Polygon*> *nResultPolyList)
{
	unsigned int i;
	unsigned int nPoints=nResultPointList->size();
	unsigned int nLines=nResultLineList->size();
	unsigned int nPolys=nResultPolyList->size();

	vector<Geometry*> *geomList=new vector<Geometry*>();
	geomList->reserve(nPoints+nLines+nPolys);

	// element geometries of the result are always in the order P,L,A
	for(i=0; i<nPoints; ++i) {
		Point *pt=(*nResultPointList)[i];
		geomList->push_back(pt);
	}
	for(i=0; i<nLines; ++i) {
		LineString *ls=(*nResultLineList)[i];
		geomList->push_back(ls);
	}
	for(i=0; i<nPolys; ++i) {
		Polygon *q=(*nResultPolyList)[i];
		geomList->push_back(q);
	}

	// build the most specific geometry possible
	Geometry *g=geomFact->buildGeometry(geomList);
	return g;
}

/*private*/
void
OverlayOp::computeOverlay(int opCode)
	//throw(TopologyException *)
{

	// copy points from input Geometries.
	// This ensures that any Point geometries
	// in the input are considered for inclusion in the result set
	copyPoints(0);
	copyPoints(1);

	// node the input Geometries
	delete arg[0]->computeSelfNodes(li,false);
	delete arg[1]->computeSelfNodes(li,false);

#if GEOS_DEBUG
	cerr<<"OverlayOp::computeOverlay: computed SelfNodes"<<endl;
#endif

	// compute intersections between edges of the two input geometries
	delete arg[0]->computeEdgeIntersections(arg[1], &li,true);

#if GEOS_DEBUG
	cerr<<"OverlayOp::computeOverlay: computed EdgeIntersections"<<endl;
	cerr<<"OverlayOp::computeOverlay: li: "<<li.toString()<<endl;
#endif


	vector<Edge*> baseSplitEdges;
	arg[0]->computeSplitEdges(&baseSplitEdges);
	arg[1]->computeSplitEdges(&baseSplitEdges);

	// add the noded edges to this result graph
	insertUniqueEdges(&baseSplitEdges);
	computeLabelsFromDepths();
	replaceCollapsedEdges();
	//Debug.println(edgeList);

    	// debugging only
    	//NodingValidator nv = new NodingValidator(edgeList.getEdges());
    	//nv.checkValid();

	graph.addEdges(edgeList.getEdges());

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

	PolygonBuilder polyBuilder(geomFact);
	
	// might throw a TopologyException *
	polyBuilder.add(&graph);

	vector<Geometry*> *gv=polyBuilder.getPolygons();
	unsigned int gvsize=gv->size();
	resultPolyList=new vector<Polygon*>(gvsize);
	for(unsigned int i=0; i<gvsize; ++i) {
		(*resultPolyList)[i]=(Polygon*)(*gv)[i];
	}
	delete gv;

	LineBuilder lineBuilder(this,geomFact,&ptLocator);
	resultLineList=lineBuilder.build(opCode);

	PointBuilder pointBuilder(this,geomFact,&ptLocator);
	resultPointList=pointBuilder.build(opCode);

	// gather the results from all calculations into a single
	// Geometry for the result set
	resultGeom=computeGeometry(resultPointList,resultLineList,resultPolyList);
#if USE_ELEVATION_MATRIX
	elevationMatrix->elevate(resultGeom);
#endif // USE_ELEVATION_MATRIX
	
}

/*protected*/
void
OverlayOp::insertUniqueEdge(Edge *e)
{
	//Debug.println(e);
#if GEOS_DEBUG
	cerr<<"OverlayOp::insertUniqueEdge("<<e->print()<<")"<<endl;
#endif

	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
	Edge *existingEdge = edgeList.findEqualEdge(e);

	// If an identical edge already exists, simply update its label
	if (existingEdge) {
#if GEOS_DEBUG
		cerr<<"  found identical edge, should merge Z"<<endl;
#endif
		Label *existingLabel=existingEdge->getLabel();

		Label *labelToMerge=e->getLabel();

		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (!existingEdge->isPointwiseEqual(e)) {
//			labelToMerge=new Label(e->getLabel());
			labelToMerge->flip();
		}
		Depth &depth=existingEdge->getDepth();
		// if this is the first duplicate found for this edge, initialize the depths
		///*
		if (depth.isNull()) {
			depth.add(*existingLabel);
		}
		//*/
		depth.add(*labelToMerge);

		existingLabel->merge(*labelToMerge);
		//Debug.print("inserted edge: "); Debug.println(e);
		//Debug.print("existing edge: "); Debug.println(existingEdge);
		dupEdges.push_back(e);
	} else {  // no matching existing edge was found
#if GEOS_DEBUG
		cerr<<"  no matching existing edge"<<endl;
#endif
		// add this new edge to the list of edges in this graph
		//e.setName(name+edges.size());
		//e.getDepth().add(e.getLabel());
		edgeList.add(e);
	}
}

/*private*/
void
OverlayOp::computeLabelsFromDepths()
{
	for(unsigned int j=0, s=edgeList.getEdges().size(); j<s; ++j)
	{
		Edge *e=edgeList.get(j);
		Label *lbl=e->getLabel();
		Depth &depth=e->getDepth();

		/*
		 * Only check edges for which there were duplicates,
		 * since these are the only ones which might
		 * be the result of dimensional collapses.
		 */
		if (depth.isNull()) continue;

		depth.normalize();
		for (int i=0;i<2;i++)
		{
			if (!lbl->isNull(i) && lbl->isArea() && !depth.isNull(i))
			{
				/*
				 * if the depths are equal, this edge is the result of
				 * the dimensional collapse of two or more edges.
				 * It has the same location on both sides of the edge,
				 * so it has collapsed to a line.
				 */
				if (depth.getDelta(i)==0) {
					lbl->toLine(i);
				} else {
					/*
					 * This edge may be the result of a dimensional collapse,
					 * but it still has different locations on both sides.  The
					 * label of the edge must be updated to reflect the resultant
					 * side locations indicated by the depth values.
					 */
					assert(!depth.isNull(i,Position::LEFT)); // depth of LEFT side has not been initialized
					lbl->setLocation(i,Position::LEFT,depth.getLocation(i,Position::LEFT));
					assert(!depth.isNull(i,Position::RIGHT)); // depth of RIGHT side has not been initialized
					lbl->setLocation(i,Position::RIGHT,depth.getLocation(i,Position::RIGHT));
				}
			}
		}
	}
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.63  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 * Revision 1.62  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.61  2006/03/09 15:50:27  strk
 * Fixed debugging lines, added missing header
 *
 * Revision 1.60  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.59  2006/03/06 12:11:48  strk
 * precision.h => geos/precision.h (#48)
 *
 * Revision 1.58  2006/03/06 11:17:08  strk
 * precision reducing overlayOp made compile-time optional
 *
 * Revision 1.57  2006/03/03 14:01:12  strk
 * Experimental precision-reducing overlayOp
 *
 * Revision 1.56  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.55  2006/03/02 14:34:43  strk
 * GeometryGraphOperation::li made a non-static member, and not more a pointer
 *
 * Revision 1.54  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.53  2006/03/02 09:51:36  strk
 * Fixes in DEBUG lines (bug#42)
 *
 * Revision 1.52  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.51  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.50  2005/12/07 19:18:23  strk
 * Changed PlanarGraph::addEdges and EdgeList::addAll to take
 * a const vector by reference rather then a non-const vector by
 * pointer.
 * Optimized polygon vector allocations in OverlayOp::computeOverlay.
 *
 * Revision 1.49  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.48  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.47  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.46  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.45  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.44  2005/11/08 11:33:10  strk
 * comments cleanup
 *
 * Revision 1.43  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.42  2005/11/04 08:28:29  strk
 * Ported speedup of OverlayOp::insertUniqueEdge() from JTS-1.7 (rev 1.23)
 * Updated NEWS file.
 *
 * Revision 1.41  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.40  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.39  2005/04/21 11:16:14  strk
 * Removed useless Coordinate copies in mergeZ() - patch by Safe Software
 *
 * Revision 1.38  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
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

