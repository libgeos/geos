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

#include <geos/opBuffer.h>
#include <geos/profiler.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

/**
* Compute the change in depth as an edge is crossed from R to L
*/
int
BufferBuilder::depthDelta(Label *label)
{
	int lLoc=label->getLocation(0, Position::LEFT);
	int rLoc=label->getLocation(0, Position::RIGHT);
	if (lLoc== Location::INTERIOR && rLoc== Location::EXTERIOR)
		return 1;
	else if (lLoc== Location::EXTERIOR && rLoc== Location::INTERIOR)
		return -1;
	return 0;
}

static RobustCGAlgorithms rCGA;
CGAlgorithms *BufferBuilder::cga=&rCGA;

/**
 * Creates a new BufferBuilder
 */
BufferBuilder::BufferBuilder():
	quadrantSegments(OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS),
	endCapStyle(BufferOp::CAP_ROUND),
	workingPrecisionModel(NULL),
	edgeList(new EdgeList())
{
}

BufferBuilder::~BufferBuilder()
{
	delete edgeList;
	for (unsigned int i=0; i<newLabels.size(); i++)
		delete newLabels[i];
}

/**
 * Sets the number of segments used to approximate a angle fillet
 *
 * @param quadrantSegments the number of segments in a fillet for a quadrant
 */
void
BufferBuilder::setQuadrantSegments(int nQuadrantSegments)
{
	quadrantSegments=nQuadrantSegments;
}

/**
 * Sets the precision model to use during the curve computation and noding,
 * if it is different to the precision model of the Geometry->
 * If the precision model is less than the precision of the Geometry
 * precision model,
 * the Geometry must have previously been rounded to that precision->
 *
 * @param pm the precision model to use
 */
void
BufferBuilder::setWorkingPrecisionModel(PrecisionModel *pm)
{
	workingPrecisionModel=pm;
}

void
BufferBuilder::setEndCapStyle(int nEndCapStyle)
{
	endCapStyle=nEndCapStyle;
}

Geometry*
BufferBuilder::buffer(Geometry *g, double distance)
	// throw(GEOSException *)
{
	const PrecisionModel *precisionModel=workingPrecisionModel;
	if (precisionModel==NULL)
		precisionModel=g->getPrecisionModel();

	// factory must be the same as the one used by the input
	geomFact=g->getFactory();
	OffsetCurveBuilder curveBuilder(precisionModel, quadrantSegments);
	curveBuilder.setEndCapStyle(endCapStyle);
	OffsetCurveSetBuilder curveSetBuilder(g, distance, &curveBuilder);
	vector<SegmentString*> *bufferSegStrList=curveSetBuilder.getCurves();
	// short-circuit test
	if (bufferSegStrList->size()<=0) {
		Geometry *emptyGeom=geomFact->createGeometryCollection(NULL);
		return emptyGeom;
	}

#if DEBUG
	cerr<<"BufferBuilder::buffer computing NodedEdges"<<endl;
#endif
#if PROFILE
	profiler->start("BufferBuilder::computeNodedEdges()");
#endif
	computeNodedEdges(bufferSegStrList, precisionModel);
#if PROFILE
	profiler->stop("BufferBuilder::computeNodedEdges()");
#endif
#if DEBUG
	cerr<<"BufferBuilder::buffer finished computing NodedEdges"<<endl;
#endif

	Geometry* resultGeom=NULL;
	vector<Geometry*> *resultPolyList=NULL;
	vector<BufferSubgraph*> *subgraphList=NULL;
	try {
		PlanarGraph graph(new OverlayNodeFactory());
		graph.addEdges(edgeList->getEdges());
		subgraphList=createSubgraphs(&graph);
		PolygonBuilder polyBuilder(geomFact,cga);
		buildSubgraphs(subgraphList, &polyBuilder);
		resultPolyList=polyBuilder.getPolygons();
		resultGeom=geomFact->buildGeometry(resultPolyList);
	} catch (GEOSException *exc) {
		for (unsigned int i=0; i<subgraphList->size(); i++)
			delete (*subgraphList)[i];
		delete subgraphList;
		//for (unsigned int i=0; i<resultPolyList->size(); i++)
		//	delete (*resultPolyList)[i];
		//delete resultPolyList;
		throw;
	} 
	for (unsigned int i=0; i<subgraphList->size(); i++)
		delete (*subgraphList)[i];
	delete subgraphList;
	return resultGeom;
}

void
BufferBuilder::computeNodedEdges(vector<SegmentString*> *bufferSegStrList, const PrecisionModel *precisionModel)
	// throw(GEOSException *)
{
	//BufferCurveGraphNoder noder=new BufferCurveGraphNoder(geomFact->getPrecisionModel());
	IteratedNoder noder(precisionModel);
	vector<SegmentString*> *nodedSegStrings = NULL;
	
	try 
	{
#if DEBUG
		cerr<<"BufferBuilder::computeNodedEdges: getting nodedSegString"<<endl;
#endif
		nodedSegStrings=noder.node(bufferSegStrList);
#if DEBUG
		cerr<<"BufferBuilder::computeNodedEdges: done getting nodedSegString"<<endl;
#endif

		// DEBUGGING ONLY
		//BufferDebug->saveEdges(nodedEdges, "run" + BufferDebug->runCount + "_nodedEdges");
#if DEBUG
		cerr<<"BufferBuilder::computeNodedEdges: setting label for "<<nodedSegStrings->size()<<" nodedSegStrings"<<endl;
#endif
#if PROFILE
	profiler->start("BufferBuilder::computeNodedEdges: labeling");
#endif
		for (unsigned int i=0;i<nodedSegStrings->size();i++) {
			SegmentString *segStr=(*nodedSegStrings)[i];
			Label *oldLabel=(Label*) segStr->getContext();
			Edge *edge=new Edge((CoordinateSequence*) segStr->getCoordinates(), new Label(oldLabel));
			insertEdge(edge);
		}
#if PROFILE
	profiler->stop("BufferBuilder::computeNodedEdges: labeling");
#endif
#if DEBUG
		cerr<<"BufferBuilder::computeNodedEdges: labeling for "<<nodedSegStrings->size()<<" nodedSegStrings done"<<endl;
#endif
		//saveEdges(edgeList->getEdges(), "run" + runCount + "_collapsedEdges");
	} catch (...) {
		delete nodedSegStrings;
		throw;
	} 
	delete nodedSegStrings;
}


/**
 * Inserted edges are checked to see if an identical edge already exists->
 * If so, the edge is not inserted, but its label is merged
 * with the existing edge->
 */
void
BufferBuilder::insertEdge(Edge *e)
{
	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
#if PROFILE
	profiler->start("EdgeList::findEqualEdge()");
#endif
	Edge *existingEdge=edgeList->findEqualEdge(e);
#if PROFILE
	profiler->stop("EdgeList::findEqualEdge()");
#endif
	// If an identical edge already exists, simply update its label
	if (existingEdge != NULL) {
		Label *existingLabel=existingEdge->getLabel();
		Label *labelToMerge=e->getLabel();

		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (! existingEdge->isPointwiseEqual(e))
		{
			labelToMerge=new Label(e->getLabel());
			labelToMerge->flip();
			newLabels.push_back(labelToMerge);
		}
		existingLabel->merge(labelToMerge);
		// compute new depth delta of sum of edges
		int mergeDelta=depthDelta(labelToMerge);
		int existingDelta=existingEdge->getDepthDelta();
		int newDelta=existingDelta + mergeDelta;
		existingEdge->setDepthDelta(newDelta);
		delete e;
	} else {   // no matching existing edge was found
		// add this new edge to the list of edges in this graph
		//e->setName(name + edges->size());
		edgeList->add(e);
		e->setDepthDelta(depthDelta(e->getLabel()));
	}
}

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second) {
	if (first->compareTo(second)>=0)
		return true;
	else
		return false;
}

vector<BufferSubgraph*>*
BufferBuilder::createSubgraphs(PlanarGraph *graph)
{
	vector<BufferSubgraph*> *subgraphList=new vector<BufferSubgraph*>();
	vector<Node*> *n=graph->getNodes();
	for (unsigned int i=0;i<n->size();i++) {
		Node *node=(*n)[i];
		if (!node->isVisited()) {
			BufferSubgraph *subgraph=new BufferSubgraph(cga);
			subgraph->create(node);
			subgraphList->push_back(subgraph);
		}
	}
	delete n;
	/**
	* Sort the subgraphs in descending order of their rightmost coordinate->
	* This ensures that when the Polygons for the subgraphs are built,
	* subgraphs for shells will have been built before the subgraphs for
	* any holes they contain->
	*/
	sort(subgraphList->begin(),subgraphList->end(),BufferSubgraphGT);
	return subgraphList;
}

/**
* Completes the building of the input subgraphs by depth-labelling them,
* and adds them to the PolygonBuilder->
* The subgraph list must be sorted in rightmost-coordinate order->
*
* @param subgraphList the subgraphs to build
* @param polyBuilder the PolygonBuilder which will build the final polygons
*/
void
BufferBuilder::buildSubgraphs(vector<BufferSubgraph*> *subgraphList,PolygonBuilder *polyBuilder)
{
	vector<BufferSubgraph*> processedGraphs;
	for (unsigned int i=0;i<subgraphList->size();i++) {
		BufferSubgraph *subgraph=(*subgraphList)[i];
		Coordinate *p=subgraph->getRightmostCoordinate();
		SubgraphDepthLocater locater=SubgraphDepthLocater(&processedGraphs);
		int outsideDepth=locater.getDepth(*p);
		subgraph->computeDepth(outsideDepth);
		subgraph->findResultEdges();
		processedGraphs.push_back(subgraph);
		polyBuilder->add(subgraph->getDirectedEdges(), subgraph->getNodes());
	}
}
}

/**********************************************************************
 * $Log$
 * Revision 1.25  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.24  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.23  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.22  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.21  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.20  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.19  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.18  2004/06/15 20:10:31  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.17  2004/06/15 20:01:58  strk
 * Empty geometry creation call made using NULL instead of newly created empty vector (will be faster)
 *
 * Revision 1.16  2004/05/26 13:12:58  strk
 * Removed try/catch block from ::buildSubgraphs
 *
 * Revision 1.15  2004/05/26 09:49:03  strk
 * PlanarGraph made local to ::buffer instead of Class private.
 *
 * Revision 1.14  2004/05/05 16:57:48  strk
 * Rewritten static cga allocation to avoid copy constructor calls.
 *
 * Revision 1.13  2004/05/05 16:36:46  strk
 * Avoid use of copy c'tors on local objects initializzation
 *
 * Revision 1.12  2004/05/05 13:08:01  strk
 * Leaks fixed, explicit allocations/deallocations reduced.
 *
 * Revision 1.11  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.10  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.9  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.8  2004/05/03 10:43:43  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.7  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.6  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.5  2004/04/20 10:58:04  strk
 * More memory leaks removed.
 *
 * Revision 1.4  2004/04/19 15:14:46  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.3  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.2  2004/04/14 08:38:52  strk
 * BufferBuilder constructor missed to initialize workingPrecisionModel
 *
 * Revision 1.1  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

