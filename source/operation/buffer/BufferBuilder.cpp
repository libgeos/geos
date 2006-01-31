/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
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

//static CGAlgorithms rCGA;
//CGAlgorithms *BufferBuilder::cga=&rCGA;

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
#if DEBUG
	cerr<<"OffsetCurveSetBuilder got "<<bufferSegStrList->size()<<" curves"<<endl;
#endif
	// short-circuit test
	if (bufferSegStrList->size()<=0) {
		Geometry *emptyGeom=geomFact->createGeometryCollection(NULL);
		return emptyGeom;
	}

#if DEBUG
	cerr<<"BufferBuilder::buffer computing NodedEdges"<<endl;
#endif
#if PROFILE
	static Profile *prof=profiler->get("BufferBuilder::computeNodedEdges()");
	prof->start();
#endif
	computeNodedEdges(bufferSegStrList, precisionModel);
#if PROFILE
	prof->stop();
#endif
#if DEBUG
	cerr<<"BufferBuilder::buffer finished computing NodedEdges"<<endl;
#endif

	Geometry* resultGeom=NULL;
	vector<Geometry*> *resultPolyList=NULL;
	vector<BufferSubgraph*> *subgraphList=NULL;
	try {
		PlanarGraph graph(OverlayNodeFactory::instance());
		graph.addEdges(edgeList->getEdges());
		subgraphList=createSubgraphs(&graph);
#if DEBUG
	cerr<<"Created "<<subgraphList->size()<<" subgraphs"<<endl;
#endif
		PolygonBuilder polyBuilder(geomFact);
		buildSubgraphs(subgraphList, &polyBuilder);
		resultPolyList=polyBuilder.getPolygons();
#if DEBUG
	cerr<<"PolygonBuilder got "<<resultPolyList->size()<<" polygons"<<endl;
#endif
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
		static Profile *prof = profiler->get("BufferBuilder::computeNodedEdges: labeling");
		prof->start();
#endif
		for (unsigned int i=0;i<nodedSegStrings->size();i++) {
			SegmentString *segStr=(*nodedSegStrings)[i];
			Label *oldLabel=(Label*) segStr->getContext();
			Edge *edge=new Edge((CoordinateSequence*) segStr->getCoordinates(), new Label(*oldLabel));
			insertEdge(edge);
		}
#if PROFILE
		prof->stop();
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
			labelToMerge=new Label(*(e->getLabel()));
			labelToMerge->flip();
			newLabels.push_back(labelToMerge);
		}
		existingLabel->merge(*labelToMerge);
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
	if (first->compareTo(second)>0)
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
			BufferSubgraph *subgraph=new BufferSubgraph();
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.31  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.30  2005/11/21 16:03:20  strk
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
 * Revision 1.29  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.28  2005/08/22 13:31:17  strk
 * Fixed comparator functions used with STL sort() algorithm to
 * implement StrictWeakOrdering semantic.
 *
 * Revision 1.27  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.26  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
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
 * Added CoordinateArraySequenceFactory::instance() function.
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

