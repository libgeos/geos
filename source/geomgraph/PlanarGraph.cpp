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

#include <geos/geomgraph.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

CGAlgorithms* PlanarGraph::cga=new CGAlgorithms();

/**
 * For nodes in the vector, link the DirectedEdges at the node that are in the result.
 * This allows clients to link only a subset of nodes in the graph, for
 * efficiency (because they know that only a subset is of interest).
 */
void
PlanarGraph::linkResultDirectedEdges(vector<Node*> *allNodes)
	//throw(TopologyException *)
{
	vector<Node*>::iterator nodeit;
	for(nodeit=allNodes->begin(); nodeit<allNodes->end(); nodeit++)
	{
		Node *node=*nodeit;
		// this might throw an exception
		((DirectedEdgeStar*) node->getEdges())->linkResultDirectedEdges();
	}
}

PlanarGraph::PlanarGraph(const NodeFactory &nodeFact):
	edges(new vector<Edge*>()),
	nodes(new NodeMap(nodeFact)),
	edgeEndList(new vector<EdgeEnd*>())
{

}

PlanarGraph::PlanarGraph():
	edges(new vector<Edge*>()),
	nodes(new NodeMap(NodeFactory::instance())),
	edgeEndList(new vector<EdgeEnd*>())
{
}

PlanarGraph::~PlanarGraph()
{
	delete nodes;
	int i;
	for(i=0;i<(int)edges->size();i++) {
		delete (*edges)[i];
	}
	delete edges;
	for(i=0;i<(int)edgeEndList->size();i++) {
		delete (*edgeEndList)[i];
	}
	delete edgeEndList;
}

vector<Edge*>::iterator
PlanarGraph::getEdgeIterator()
{
	return edges->begin();
}

vector<EdgeEnd*> *
PlanarGraph::getEdgeEnds()
{
	return edgeEndList;
}

bool
PlanarGraph::isBoundaryNode(int geomIndex, const Coordinate& coord)
{
	Node *node=nodes->find(coord);
	if (node==NULL) return false;
	Label *label=node->getLabel();
	if (label!=NULL && label->getLocation(geomIndex)==Location::BOUNDARY) return true;
	return false;
}

void
PlanarGraph::insertEdge(Edge *e)
{
	edges->push_back(e);
}

void
PlanarGraph::add(EdgeEnd *e)
{
	nodes->add(e);
	edgeEndList->push_back(e);
}

NodeMap::iterator
PlanarGraph::getNodeIterator()
{
	return nodes->begin();
}

vector<Node*>*
PlanarGraph::getNodes()
{
	vector<Node*> *values=new vector<Node*>();
	NodeMap::iterator it=nodes->nodeMap.begin();
	while(it!=nodes->nodeMap.end()) {
		values->push_back(it->second);
		it++;
	}
	return values;
}

// arg cannot be const, NodeMap::addNode will
// occasionally label-merge first arg.
Node*
PlanarGraph::addNode(Node *node)
{
#if DEBUG
	cerr<<"PlanarGraph::addNode(Node * "<<node->print()<<")"<<endl;
#endif
	return nodes->addNode(node);
}

Node*
PlanarGraph::addNode(const Coordinate& coord)
{
#if DEBUG
	cerr<<"PlanarGraph::addNode(Coordinate& "<<coord.toString()<<")"<<endl;
#endif
	return nodes->addNode(coord);
}

/*
 * @return the node if found; null otherwise
 */
Node*
PlanarGraph::find(Coordinate& coord)
{
	return nodes->find(coord);
}

/*
 * Add a set of edges to the graph.  For each edge two DirectedEdges
 * will be created.  DirectedEdges are NOT linked by this method.
 */
void
PlanarGraph::addEdges(const vector<Edge*> &edgesToAdd)
{
	// create all the nodes for the edges
	for (vector<Edge*>::const_iterator it=edgesToAdd.begin(),
		endIt=edgesToAdd.end(); it!=endIt; ++it)
	{
		Edge *e=*it;
		edges->push_back(e);
		DirectedEdge *de1=new DirectedEdge(e, true);
		DirectedEdge *de2=new DirectedEdge(e, false);
		de1->setSym(de2);
		de2->setSym(de1);
		add(de1);
		add(de2);
	}
}

/*
 * Link the DirectedEdges at the nodes of the graph.
 * This allows clients to link only a subset of nodes in the graph, for
 * efficiency (because they know that only a subset is of interest).
 */
void
PlanarGraph::linkResultDirectedEdges()
{
#if DEBUG
	cerr<<"PlanarGraph::linkResultDirectedEdges called"<<endl;
#endif
	NodeMap::iterator nodeit=nodes->nodeMap.begin();
	for (;nodeit!=nodes->nodeMap.end();nodeit++) {
		Node *node=nodeit->second;
		((DirectedEdgeStar*)node->getEdges())->linkResultDirectedEdges();
	}
}

/*
 * Link the DirectedEdges at the nodes of the graph.
 * This allows clients to link only a subset of nodes in the graph, for
 * efficiency (because they know that only a subset is of interest).
 */
void
PlanarGraph::linkAllDirectedEdges()
{
#if DEBUG
	cerr<<"PlanarGraph::linkAllDirectedEdges called"<<endl;
#endif
	NodeMap::iterator nodeit=nodes->nodeMap.begin();
	for (;nodeit!=nodes->nodeMap.end();nodeit++)
	{
		Node *node=nodeit->second;
		EdgeEndStar *ees=node->getEdges();
		DirectedEdgeStar *des=dynamic_cast<DirectedEdgeStar *>(ees);
		Assert::isTrue(des!=NULL, "Unespected non-DirectedEdgeStar in node");
		des->linkAllDirectedEdges();
	}
}

/*
 * Returns the EdgeEnd which has edge e as its base edge
 * (MD 18 Feb 2002 - this should return a pair of edges)
 *
 * @return the edge, if found
 *    <code>null</code> if the edge was not found
 */
EdgeEnd*
PlanarGraph::findEdgeEnd(Edge *e)
{
	for (vector<EdgeEnd*>::iterator i=getEdgeEnds()->begin();i<getEdgeEnds()->end();i++) {
		EdgeEnd *ee=*i;
		if (ee->getEdge()==e)
			return ee;
	}
	return NULL;
}

/*
 * Returns the edge whose first two coordinates are p0 and p1
 *
 * @return the edge, if found
 *    <code>null</code> if the edge was not found
 */
Edge*
PlanarGraph::findEdge(const Coordinate& p0, const Coordinate& p1)
{
    unsigned int i;
	for(i=0; i<edges->size();i++) {
//        Edge *e=edges->at(i);
        Edge *e=(*edges)[i];
		const CoordinateSequence* eCoord=e->getCoordinates();
		if (p0==eCoord->getAt(0) && p1==eCoord->getAt(1))
			return e;
	}
	return NULL;
}

/*
 * Returns the edge which starts at p0 and whose first segment is
 * parallel to p1
 *
 * @return the edge, if found
 *    <code>null</code> if the edge was not found
 */
Edge*
PlanarGraph::findEdgeInSameDirection(const Coordinate& p0, const Coordinate& p1)
{
	for(unsigned int i=0; i<edges->size();i++) {
		Edge *e=(*edges)[i];
//		Edge *e=edges->at(i);
		const CoordinateSequence* eCoord=e->getCoordinates();
		if (matchInSameDirection(p0,p1,eCoord->getAt(0),eCoord->getAt(1)))
			return e;
		if (matchInSameDirection(p0,p1,eCoord->getAt(eCoord->getSize()-1),eCoord->getAt(eCoord->getSize()-2)))
			return e;
	}
	return NULL;
}

/*
 * The coordinate pairs match if they define line segments
 * lying in the same direction.
 * E.g. the segments are parallel and in the same quadrant
 * (as opposed to parallel and opposite!).
 */
bool
PlanarGraph::matchInSameDirection(const Coordinate& p0, const Coordinate& p1, const Coordinate& ep0, const Coordinate& ep1)
{
	if (!(p0==ep0))
		return false;

	if (CGAlgorithms::computeOrientation(p0,p1,ep1)==CGAlgorithms::COLLINEAR
		&& Quadrant::quadrant(p0,p1)==Quadrant::quadrant(ep0,ep1))
			return true;
	return false;
}

string
PlanarGraph::printEdges()
{
	string out="Edges: ";
	for(unsigned int i=0;i<edges->size();i++) {
		out+="edge ";
		out+=i;
		out+=":\n";
		Edge *e=(*edges)[i];
//		Edge *e=edges->at(i);
		out+=e->print();
		out+=e->eiList.print();
	}
	return out;
}

NodeMap*
PlanarGraph::getNodeMap()
{
	return nodes;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.19  2006/01/31 19:07:34  strk
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
 * Revision 1.18  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.17  2005/12/07 19:18:23  strk
 * Changed PlanarGraph::addEdges and EdgeList::addAll to take
 * a const vector by reference rather then a non-const vector by
 * pointer.
 * Optimized polygon vector allocations in OverlayOp::computeOverlay.
 *
 * Revision 1.16  2005/11/21 16:03:20  strk
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
 * Revision 1.15  2005/11/16 22:21:45  strk
 * enforced const-correctness and use of initializer lists.
 *
 * Revision 1.14  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.13  2005/06/24 11:09:42  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.12  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.11  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.10  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.9  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.8  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.7  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.6  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.5  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.4  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.3  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.2  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.19  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.18  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.17  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/

