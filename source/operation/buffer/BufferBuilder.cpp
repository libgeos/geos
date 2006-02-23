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
 **********************************************************************
 *
 * Last port: operation/buffer/BufferBuilder.java rev. 1.21 (JTS-1.7)
 *
 **********************************************************************/

#include <cassert>
#include <geos/noding.h>
#include <geos/opBuffer.h>
#include <geos/profiler.h>

#ifndef DEBUG
#define DEBUG 0
#endif

using namespace geos::geomgraph;
using namespace geos::noding;
using namespace geos::algorithm;
using namespace geos::operation::overlay;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

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

BufferBuilder::~BufferBuilder()
{
	delete li; // could be NULL
	delete intersectionAdder;
	delete edgeList;
	for (unsigned int i=0; i<newLabels.size(); i++)
		delete newLabels[i];

	// FIXME: I think it's this object responsibility to delete
	// created Edges, OR callers must provided a container
	// for keeping track of them!
	for (unsigned int i=0; i<newEdges.size(); i++) delete newEdges[i];
}

Geometry*
BufferBuilder::buffer(const Geometry *g, double distance)
	// throw(GEOSException *)
{
	const PrecisionModel *precisionModel=workingPrecisionModel;
	if (precisionModel==NULL)
		precisionModel=g->getPrecisionModel();

	assert(precisionModel);

	// factory must be the same as the one used by the input
	geomFact=g->getFactory();
	OffsetCurveBuilder curveBuilder(precisionModel, quadrantSegments);
	curveBuilder.setEndCapStyle(endCapStyle);
	OffsetCurveSetBuilder curveSetBuilder(*g, distance, curveBuilder);
	vector<SegmentString*>& bufferSegStrList=curveSetBuilder.getCurves();
#if DEBUG
	cerr<<"OffsetCurveSetBuilder got "<<bufferSegStrList.size()<<" curves"<<endl;
#endif
	// short-circuit test
	if (bufferSegStrList.size()<=0) {
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
	vector<BufferSubgraph*> subgraphList;

	try {
		PlanarGraph graph(OverlayNodeFactory::instance());
		graph.addEdges(edgeList->getEdges());

		createSubgraphs(&graph, subgraphList);
#if DEBUG
	cerr<<"Created "<<subgraphList.size()<<" subgraphs"<<endl;
#endif
		PolygonBuilder polyBuilder(geomFact);
		buildSubgraphs(&subgraphList, &polyBuilder);
		resultPolyList=polyBuilder.getPolygons();
#if DEBUG
	cerr<<"PolygonBuilder got "<<resultPolyList->size()<<" polygons"<<endl;
#endif
		resultGeom=geomFact->buildGeometry(resultPolyList);
	} catch (const GEOSException& exc) {
		for (unsigned int i=0, n=subgraphList.size(); i<n; i++)
			delete subgraphList[i];
		throw;
	} 

	for (unsigned int i=0, n=subgraphList.size(); i<n; i++)
		delete subgraphList[i];

	return resultGeom;
}

/*private*/
Noder*
BufferBuilder::getNoder(const PrecisionModel* pm)
{
	// this doesn't change workingNoder precisionModel!
	if (workingNoder != NULL) return workingNoder;

	// otherwise use a fast (but non-robust) noder
	MCIndexNoder* noder = new MCIndexNoder();

	if ( li ) // reuse existing IntersectionAdder and LineIntersector
	{
		li->setPrecisionModel(pm);
		assert(intersectionAdder!=NULL);
	}
	else
	{
		li = new LineIntersector(pm);
		intersectionAdder = new IntersectionAdder(*li);
	}

	noder->setSegmentIntersector(intersectionAdder);

	return noder;

#if 0
	Noder noder = new IteratedNoder(precisionModel);
	Noder noder = new SimpleSnapRounder(precisionModel);
	Noder noder = new MCIndexSnapRounder(precisionModel);
	Noder noder = new ScaledNoder(
		new MCIndexSnapRounder(new PrecisionModel(1.0)),
			precisionModel.getScale());
#endif


}

/* private */
void
BufferBuilder::computeNodedEdges(SegmentString::NonConstVect& bufferSegStrList,
		const PrecisionModel *precisionModel) // throw(GEOSException)
{
	Noder *noder = getNoder(precisionModel);

	noder->computeNodes(&bufferSegStrList);

	SegmentString::NonConstVect* nodedSegStrings = \
			noder->getNodedSubstrings();


	for (SegmentString::NonConstVect::iterator
		i=nodedSegStrings->begin(), e=nodedSegStrings->end();
		i!=e;
		++i)
	{
		SegmentString* segStr = *i;
		const Label* oldLabel = (Label*)(segStr->getData());

		// we need to clone SegmentString coordinates
		// as Edge will take ownership of them
		// TODO: find a way to transfer ownership instead
		// Who will own the edge ? FIXME: find out and handle that!
		Edge* edge = new Edge(segStr->getCoordinates()->clone(),
				new Label(*oldLabel));

		// will take care of the Edge ownership
		insertEdge(edge);
	}

	if ( nodedSegStrings != &bufferSegStrList )
	{
		delete nodedSegStrings;
	}

	if ( noder != workingNoder ) delete noder;
}

/*private*/
void
BufferBuilder::insertEdge(Edge *e)
{
	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
	Edge *existingEdge=edgeList->findEqualEdge(e);
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

		// we have memory release responsibility 
		delete e;

	} else {   // no matching existing edge was found

		// add this new edge to the list of edges in this graph
		edgeList->add(e);

		e->setDepthDelta(depthDelta(e->getLabel()));

		// FIXME: I think we should have memory release responsibility.
		// Verify how to do that, as it seems it Edges will be deleted
		// *at least* by PlanarGraph and who knows who else.
		//newEdges.push_back(e);

	}
}

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second) {
	if (first->compareTo(second)>0)
		return true;
	else
		return false;
}

/*private*/
void
BufferBuilder::createSubgraphs(PlanarGraph *graph, vector<BufferSubgraph*>& subgraphList)
{
	vector<Node*>* nodes = graph->getNodes();
	for (unsigned int i=0, n=nodes->size(); i<n; i++) {
		Node *node=(*nodes)[i];
		if (!node->isVisited()) {
			BufferSubgraph *subgraph=new BufferSubgraph();
			subgraph->create(node);
			subgraphList.push_back(subgraph);
		}
	}
	delete nodes;

	/*
	 * Sort the subgraphs in descending order of their rightmost coordinate
	 * This ensures that when the Polygons for the subgraphs are built,
	 * subgraphs for shells will have been built before the subgraphs for
	 * any holes they contain
	 */
	sort(subgraphList.begin(),subgraphList.end(),BufferSubgraphGT);
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

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.37  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.36  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.35  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.34  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.33  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.32  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
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

