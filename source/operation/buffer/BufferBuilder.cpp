/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2005-2007 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferBuilder.java rev. 1.30 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/overlay/PolygonBuilder.h> 
#include <geos/operation/overlay/OverlayNodeFactory.h> 
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Location.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>
#include <geos/operation/buffer/OffsetCurveSetBuilder.h>
#include <geos/operation/buffer/BufferSubgraph.h>
#include <geos/operation/buffer/SubgraphDepthLocater.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/geomgraph/Position.h>
#include <geos/geomgraph/PlanarGraph.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Edge.h>
#include <geos/util/GEOSException.h>
#include <geos/profiler.h>

#include <cassert>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iostream>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifndef JTS_DEBUG 
#define JTS_DEBUG 0
#endif

//using namespace std;
using namespace geos::geom;
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
	//delete edgeList;
	for (size_t i=0; i<newLabels.size(); i++)
		delete newLabels[i];
}

/*public*/
Geometry*
BufferBuilder::buffer(const Geometry *g, double distance)
	// throw(GEOSException *)
{
	const PrecisionModel *precisionModel=workingPrecisionModel;
	if (precisionModel==NULL)
		precisionModel=g->getPrecisionModel();

	assert(precisionModel);
	assert(g);

	// factory must be the same as the one used by the input
	geomFact=g->getFactory();

	OffsetCurveBuilder curveBuilder(precisionModel, bufParams);
	OffsetCurveSetBuilder curveSetBuilder(*g, distance, curveBuilder);

	std::vector<SegmentString*>& bufferSegStrList=curveSetBuilder.getCurves();

#if GEOS_DEBUG
	std::cerr << "OffsetCurveSetBuilder got " << bufferSegStrList.size()
	          << " curves" << std::endl;
#endif
	// short-circuit test
	if (bufferSegStrList.size()<=0) {
		return createEmptyResultGeometry();
	}

#if GEOS_DEBUG
	std::cerr<<"BufferBuilder::buffer computing NodedEdges"<<std::endl;
#endif

#if JTS_DEBUG
std::cerr << "before noding: SegStr # " << bufferSegStrList.size() << std::endl;
for (size_t i = 0, n=bufferSegStrList.size(); i<n; i++)
{
 	SegmentString* segStr = bufferSegStrList[i];
	std::cerr << "SegStr " << i << ": pts # " << segStr->size()
		<< " nodes # " << segStr->getNodeList().size()
		<< std::endl;
}
#endif

	computeNodedEdges(bufferSegStrList, precisionModel);

#if JTS_DEBUG
std::cerr << "after noding: SegStr # " << bufferSegStrList.size() << std::endl;
for (size_t i = 0, n=bufferSegStrList.size(); i<n; i++)
{
 	SegmentString* segStr = bufferSegStrList[i];
	std::cerr << "SegStr " << i << ": pts # " << segStr->size()
		<< " nodes # " << segStr->getNodeList().size()
		<< std::endl;
}
#endif

#if GEOS_DEBUG > 1
	std::cerr << std::endl << edgeList << std::endl;
#endif

	Geometry* resultGeom=NULL;
	std::auto_ptr< std::vector<Geometry*> > resultPolyList;
	std::vector<BufferSubgraph*> subgraphList;

	try {
		PlanarGraph graph(OverlayNodeFactory::instance());
		graph.addEdges(edgeList.getEdges());

		createSubgraphs(&graph, subgraphList);
#if GEOS_DEBUG
	std::cerr<<"Created "<<subgraphList.size()<<" subgraphs"<<std::endl;
#if GEOS_DEBUG > 1
	for (size_t i=0, n=subgraphList.size(); i<n; i++)
		std::cerr << std::setprecision(10) << *(subgraphList[i]) << std::endl;
#endif
#endif
		PolygonBuilder polyBuilder(geomFact);
		buildSubgraphs(subgraphList, polyBuilder);
		resultPolyList.reset( polyBuilder.getPolygons() );
#if GEOS_DEBUG
	std::cerr << "PolygonBuilder got " << resultPolyList->size()
	          << " polygons" << std::endl;
#if GEOS_DEBUG > 1
	for (size_t i=0, n=resultPolyList->size(); i<n; i++)
		std::cerr << (*resultPolyList)[i]->toString() << std::endl;
#endif
#endif
		// just in case ...
		if ( resultPolyList->empty() )
		{
			for (size_t i=0, n=subgraphList.size(); i<n; i++)
				delete subgraphList[i];
			return createEmptyResultGeometry();
		}

		// resultPolyList ownership transferred here
		resultGeom=geomFact->buildGeometry(resultPolyList.release());
	} catch (const util::GEOSException& /* exc */) {
		for (size_t i=0, n=subgraphList.size(); i<n; i++)
			delete subgraphList[i];
		throw;
	} 

	for (size_t i=0, n=subgraphList.size(); i<n; i++)
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

	MCIndexNoder* noder = new MCIndexNoder(intersectionAdder);

#if 0
	/* CoordinateArraySequence.cpp:84:
	 * virtual const geos::Coordinate& geos::CoordinateArraySequence::getAt(size_t) const:
	 * Assertion `pos<vect->size()' failed.
	 */
	//Noder* noder = new snapround::SimpleSnapRounder(*pm);

	Noder* noder = new IteratedNoder(pm);

	Noder noder = new SimpleSnapRounder(pm);
	Noder noder = new MCIndexSnapRounder(pm);
	Noder noder = new ScaledNoder(
		new MCIndexSnapRounder(new PrecisionModel(1.0)),
			pm.getScale());
#endif

	return noder;

}

/* private */
void
BufferBuilder::computeNodedEdges(SegmentString::NonConstVect& bufferSegStrList,
		const PrecisionModel *precisionModel) // throw(GEOSException)
{
	Noder* noder = getNoder( precisionModel );

	noder->computeNodes(&bufferSegStrList);

	SegmentString::NonConstVect* nodedSegStrings = \
			noder->getNodedSubstrings();


	for (SegmentString::NonConstVect::iterator
		i=nodedSegStrings->begin(), e=nodedSegStrings->end();
		i!=e;
		++i)
	{
		SegmentString* segStr = *i;
		const Label* oldLabel = static_cast<const Label*>(segStr->getData());

		CoordinateSequence* cs = CoordinateSequence::removeRepeatedPoints(segStr->getCoordinates());
		if ( cs->size() < 2 ) 
		{
			delete cs; // we need to take care of the memory here as cs is a new sequence
			return; // don't insert collapsed edges
		}
		// we need to clone SegmentString coordinates
		// as Edge will take ownership of them
		// TODO: find a way to transfer ownership instead
		// Who will own the edge ? FIXME: find out and handle that!
		Edge* edge = new Edge(cs, new Label(*oldLabel));

		// will take care of the Edge ownership
		insertUniqueEdge(edge);
	}

	if ( nodedSegStrings != &bufferSegStrList )
	{
		delete nodedSegStrings;
	}

	if ( noder != workingNoder ) delete noder;
}

/*private*/
void
BufferBuilder::insertUniqueEdge(Edge *e)
{
	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
	Edge *existingEdge=edgeList.findEqualEdge(e);
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
		edgeList.add(e);

		e->setDepthDelta(depthDelta(e->getLabel()));
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
BufferBuilder::createSubgraphs(PlanarGraph *graph, std::vector<BufferSubgraph*>& subgraphList)
{
	std::vector<Node*> nodes;
	graph->getNodes(nodes);
	for (size_t i=0, n=nodes.size(); i<n; i++) {
		Node *node=nodes[i];
		if (!node->isVisited()) {
			BufferSubgraph *subgraph=new BufferSubgraph();
			subgraph->create(node);
			subgraphList.push_back(subgraph);
		}
	}

	/*
	 * Sort the subgraphs in descending order of their rightmost coordinate
	 * This ensures that when the Polygons for the subgraphs are built,
	 * subgraphs for shells will have been built before the subgraphs for
	 * any holes they contain
	 */
    std::sort(subgraphList.begin(), subgraphList.end(), BufferSubgraphGT);
}

/*private*/
void
BufferBuilder::buildSubgraphs(const std::vector<BufferSubgraph*>& subgraphList,
		PolygonBuilder& polyBuilder)
{

#if GEOS_DEBUG
	std::cerr << __FUNCTION__ << " got " << subgraphList.size() << " subgraphs" << std::endl;
#endif
	std::vector<BufferSubgraph*> processedGraphs;
	for (size_t i=0, n=subgraphList.size(); i<n; i++)
	{
		BufferSubgraph *subgraph=subgraphList[i];
		Coordinate *p=subgraph->getRightmostCoordinate();
		assert(p);

#if GEOS_DEBUG
		std::cerr << " " << i << ") Subgraph[" << subgraph << "]" << std::endl;
		std::cerr << "  rightmost Coordinate " << *p;
#endif
		SubgraphDepthLocater locater(&processedGraphs);
#if GEOS_DEBUG
		std::cerr << " after SubgraphDepthLocater processedGraphs contain "
		          << processedGraphs.size()
		          << " elements" << std::endl;
#endif
		int outsideDepth=locater.getDepth(*p);
#if GEOS_DEBUG
		std::cerr << " Depth of rightmost coordinate: " << outsideDepth << std::endl;
#endif
		subgraph->computeDepth(outsideDepth);
		subgraph->findResultEdges();
#if GEOS_DEBUG
		std::cerr << " after computeDepth and findResultEdges subgraph contain:" << std::endl
		          << "   " << subgraph->getDirectedEdges()->size() << " DirecteEdges " << std::endl
		          << "   " << subgraph->getNodes()->size() << " Nodes " << std::endl;
#endif
		processedGraphs.push_back(subgraph);
#if GEOS_DEBUG
		std::cerr << " added " << subgraph << " to processedGraphs, new size is "
		          << processedGraphs.size() << std::endl;
#endif
		polyBuilder.add(subgraph->getDirectedEdges(), subgraph->getNodes());
	}
}

/*private*/
geom::Geometry*
BufferBuilder::createEmptyResultGeometry() const
{
	geom::Geometry* emptyGeom = geomFact->createPolygon(NULL, NULL);
	return emptyGeom;
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.56  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.55  2006/05/04 09:16:58  strk
 * Added JTS debugging, for comparison with JTS
 *
 * Revision 1.54  2006/03/24 09:25:02  strk
 * Bugs #77 and #76: missing <algorithm>
 *
 * Revision 1.53  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 * Revision 1.52  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.51  2006/03/15 18:57:10  strk
 * cleanups in DEBUG lines
 *
 * Revision 1.50  2006/03/15 13:03:01  strk
 * removed leftover debugging line
 *
 * Revision 1.49  2006/03/15 11:42:54  strk
 * more debugging lines, with two levels of debugging handled
 *
 * Revision 1.48  2006/03/14 16:08:21  strk
 * changed buildSubgraphs signature to use refs rather then pointers, made it const-correct. Reduced heap allocations in createSubgraphs()
 *
 * Revision 1.47  2006/03/14 14:16:52  strk
 * operator<< for BufferSubgraph, more debugging calls
 *
 * Revision 1.46  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.45  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 * Revision 1.44  2006/03/10 10:44:53  strk
 * Unreferenced exception objects cleanup (#52)
 *
 * Revision 1.43  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.42  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.41  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.40  2006/02/28 17:44:27  strk
 * Added a check in SegmentNode::addSplitEdge to prevent attempts
 * to build SegmentString with less then 2 points.
 * This is a temporary fix for the buffer.xml assertion failure, temporary
 * as Martin Davis review would really be needed there.
 *
 * Revision 1.39  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.38  2006/02/23 20:05:21  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
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
 **********************************************************************/

