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
#include <geos/util.h>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geomgraph::index;
using namespace geos::algorithm;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*
 * This method implements the Boundary Determination Rule
 * for determining whether
 * a component (node or edge) that appears multiple times in elements
 * of a MultiGeometry is in the boundary or the interior of the Geometry
 * 
 * The SFS uses the "Mod-2 Rule", which this function implements
 * 
 * An alternative (and possibly more intuitive) rule would be
 * the "At Most One Rule":
 *    isInBoundary = (componentCount == 1)
 */
bool
GeometryGraph::isInBoundary(int boundaryCount)
{
	// the "Mod-2 Rule"
	return boundaryCount%2==1;
}

int
GeometryGraph::determineBoundary(int boundaryCount)
{
	return isInBoundary(boundaryCount)?Location::BOUNDARY : Location::INTERIOR;
}


EdgeSetIntersector*
GeometryGraph::createEdgeSetIntersector()
{
	// various options for computing intersections, from slowest to fastest

	//private EdgeSetIntersector esi = new SimpleEdgeSetIntersector();
	//private EdgeSetIntersector esi = new MonotoneChainIntersector();
	//private EdgeSetIntersector esi = new NonReversingChainIntersector();
	//private EdgeSetIntersector esi = new SimpleSweepLineIntersector();
	//private EdgeSetIntersector esi = new MCSweepLineIntersector();

	//return new SimpleEdgeSetIntersector();
	return new SimpleMCSweepLineIntersector();
}

/*public*/
vector<Node*>*
GeometryGraph::getBoundaryNodes()
{
	if ( ! boundaryNodes.get() )
	{
		boundaryNodes.reset(new vector<Node*>());
		getBoundaryNodes(*(boundaryNodes.get()));
	}
	return boundaryNodes.get();
}

/*public*/
CoordinateSequence*
GeometryGraph::getBoundaryPoints()
{

	if ( ! boundaryPoints.get() ) 
	{
		// Collection will be destroied by GeometryGraph dtor
		vector<Node*>* coll = getBoundaryNodes();
		boundaryPoints.reset(new CoordinateArraySequence(coll->size()));
		unsigned int i=0;
		for (vector<Node*>::iterator it=coll->begin(), endIt=coll->end();
			it!=endIt; ++it)
		{
			Node *node=*it;
			boundaryPoints->setAt(node->getCoordinate(), i++);
		}
	}

	// We keep ownership of this, will be destroyed by destructor
	return boundaryPoints.get();
}

Edge*
GeometryGraph::findEdge(const LineString *line)
{
	return lineEdgeMap.find(line)->second;
}

void
GeometryGraph::computeSplitEdges(vector<Edge*> *edgelist)
{
#if GEOS_DEBUG
	cerr<<"["<<this<<"] GeometryGraph::computeSplitEdges() scanning "<<edges->size()<<" local and "<<edgelist->size()<<" provided edges"<<endl;
#endif
	for (vector<Edge*>::iterator i=edges->begin(), endIt=edges->end();
		i!=endIt; ++i)
	{
		Edge *e=*i;
#if GEOS_DEBUG
		cerr<<"   "<<e->print()<<" adding split edges from arg"<<endl;
#endif
		e->eiList.addSplitEdges(edgelist);
	}
}

void
GeometryGraph::add(const Geometry *g)
	//throw (UnsupportedOperationException *)
{
	if (g->isEmpty()) return;
	// check if this Geometry should obey the Boundary Determination Rule
	// all collections except MultiPolygons obey the rule
	if ((typeid(*g)==typeid(GeometryCollection)) ||
			   (typeid(*g)==typeid(MultiPoint)) ||
			   (typeid(*g)==typeid(MultiLineString)) &&
			   !(typeid(*g)==typeid(MultiPolygon)))
			useBoundaryDeterminationRule=true;
	if (typeid(*g)==typeid(Polygon))
		addPolygon((Polygon*) g);
	else if (typeid(*g)==typeid(LineString))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(LinearRing))
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
		string out=typeid(*g).name();
		throw util::UnsupportedOperationException("GeometryGraph::add(Geometry *): unknown geometry type: "+out);
	}
}

void
GeometryGraph::addCollection(const GeometryCollection *gc)
{
	for (int i=0;i<gc->getNumGeometries();i++) {
		const Geometry *g=gc->getGeometryN(i);
		add(g);
	}
}

/*
 * Add a Point to the graph.
 */
void
GeometryGraph::addPoint(const Point *p)
{
	const Coordinate& coord=*(p->getCoordinate());
	insertPoint(argIndex, coord, Location::INTERIOR);
}

/*
 * The left and right topological location arguments assume that the ring
 * is oriented CW.
 * If the ring is in the opposite orientation,
 * the left and right locations must be interchanged.
 */
void
GeometryGraph::addPolygonRing(const LinearRing *lr, int cwLeft, int cwRight)
	// throw IllegalArgumentException (see below)
{
	const CoordinateSequence *lrcl;
	lrcl = lr->getCoordinatesRO();
	CoordinateSequence* coord=CoordinateSequence::removeRepeatedPoints(lrcl);
	if (coord->getSize()<4) {
		hasTooFewPointsVar=true;
		invalidPoint=coord->getAt(0); // its now a Coordinate
		delete coord;
		return;
	}
	int left=cwLeft;
	int right=cwRight;

	/*
	 * the isCCW call might throw an
	 * IllegalArgumentException if degenerate ring does
	 * not contain 3 distinct points.
	 */
	try
	{
		if (CGAlgorithms::isCCW(coord)) {
			left=cwRight;
			right=cwLeft;
		}
	}
	catch(...)
	{
		delete coord;
		throw;
	}

	Edge *e=new Edge(coord,new Label(argIndex,Location::BOUNDARY,left,right));
	lineEdgeMap[lr]=e;
	insertEdge(e);
	insertPoint(argIndex,coord->getAt(0), Location::BOUNDARY);
}

void
GeometryGraph::addPolygon(const Polygon *p)
{
	addPolygonRing((LinearRing*) p->getExteriorRing(),Location::EXTERIOR,Location::INTERIOR);
	for (int i=0;i<p->getNumInteriorRing();i++) {
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CW)
		addPolygonRing((LinearRing*) p->getInteriorRingN(i),Location::INTERIOR,Location::EXTERIOR);
	}
}

void
GeometryGraph::addLineString(const LineString *line)
{
	CoordinateSequence* coord=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());
	if(coord->getSize()<2) {
		hasTooFewPointsVar=true;
		invalidPoint=coord->getAt(0);
		delete coord;
		return;
	}

	Edge *e=new Edge(coord,new Label(argIndex,Location::INTERIOR));
	lineEdgeMap[line]=e;
	insertEdge(e);

	/*
	 * Add the boundary points of the LineString, if any.
	 * Even if the LineString is closed, add both points as if they
	 * were endpoints.
	 * This allows for the case that the node already exists and is
	 * a boundary point.
	 */
	assert(coord->size() >= 2); // found LineString with single point
	insertBoundaryPoint(argIndex, coord->getAt(0));
	insertBoundaryPoint(argIndex, coord->getAt(coord->getSize()-1));
}

/*
 * Add an Edge computed externally.  The label on the Edge is assumed
 * to be correct.
 */
void
GeometryGraph::addEdge(Edge *e)
{
	insertEdge(e);
	const CoordinateSequence* coord=e->getCoordinates();
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex,coord->getAt(0),Location::BOUNDARY);
	insertPoint(argIndex,coord->getAt(coord->getSize()-1),Location::BOUNDARY);
}

/*
 * Add a point computed externally.  The point is assumed to be a
 * Point Geometry part, which has a location of INTERIOR.
 */
void
GeometryGraph::addPoint(Coordinate& pt)
{
	insertPoint(argIndex,pt,Location::INTERIOR);
}

SegmentIntersector*
GeometryGraph::computeSelfNodes(LineIntersector *li, bool computeRingSelfNodes)
{
	SegmentIntersector *si=new SegmentIntersector(li,true,false);
    	auto_ptr<EdgeSetIntersector> esi(createEdgeSetIntersector());
	// optimized test for Polygons and Rings
	if (parentGeom==NULL)
	{
		esi->computeIntersections(edges,si,true);
	}
	else if (!computeRingSelfNodes & (typeid(*parentGeom)==typeid(LinearRing)||typeid(*parentGeom)==typeid(Polygon)||typeid(*parentGeom)==typeid(MultiPolygon)))
	{
		esi->computeIntersections(edges, si, false);
	}
	else
	{
		esi->computeIntersections(edges, si, true);
	}
	//System.out.println("SegmentIntersector # tests = " + si.numTests);
	addSelfIntersectionNodes(argIndex);
	return si;
}

SegmentIntersector*
GeometryGraph::computeEdgeIntersections(GeometryGraph *g,
	LineIntersector *li, bool includeProper)
{
#if GEOS_DEBUG
	cerr<<"GeometryGraph::computeEdgeIntersections call"<<endl;
#endif
	SegmentIntersector *si=new SegmentIntersector(li, includeProper, true);
	newSegmentIntersectors.push_back(si);

	si->setBoundaryNodes(getBoundaryNodes(), g->getBoundaryNodes());
	auto_ptr<EdgeSetIntersector> esi(createEdgeSetIntersector());
	esi->computeIntersections(edges, g->edges, si);
#if GEOS_DEBUG
	cerr<<"GeometryGraph::computeEdgeIntersections returns"<<endl;
#endif
	return si;
}

void
GeometryGraph::insertPoint(int argIndex, const Coordinate& coord,
	int onLocation)
{
#if GEOS_DEBUG
	cerr<<"GeometryGraph::insertPoint("<<coord.toString()<<" called"<<endl;
#endif
	Node *n=nodes->addNode(coord);
	Label *lbl=n->getLabel();
	if (lbl==NULL) {
		n->setLabel(argIndex, onLocation);
	} else
		lbl->setLocation(argIndex, onLocation);
}

/*
 * Adds points using the mod-2 rule of SFS.  This is used to add the boundary
 * points of dim-1 geometries (Curves/MultiCurves).  According to the SFS,
 * an endpoint of a Curve is on the boundary
 * iff if it is in the boundaries of an odd number of Geometries
 */
void
GeometryGraph::insertBoundaryPoint(int argIndex,const Coordinate& coord)
{
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

void
GeometryGraph::addSelfIntersectionNodes(int argIndex)
{
	for (vector<Edge*>::iterator i=edges->begin(), endIt=edges->end();
		i!=endIt; ++i)
	{
		Edge *e=*i;
		int eLoc=e->getLabel()->getLocation(argIndex);
		EdgeIntersectionList &eiL=e->eiList;
		for (EdgeIntersectionList::iterator eiIt=eiL.begin(), eiEnd=eiL.end();
			eiIt!=eiEnd; ++eiIt)
		{
			EdgeIntersection *ei=*eiIt;
			addSelfIntersectionNode(argIndex, ei->coord, eLoc);
		}
	}
}

/*
 * Add a node for a self-intersection.
 * If the node is a potential boundary node (e.g. came from an edge which
 * is a boundary) then insert it as a potential boundary node.
 * Otherwise, just add it as a regular node.
 */
void
GeometryGraph::addSelfIntersectionNode(int argIndex,
	const Coordinate& coord, int loc)
{
	// if this node is already a boundary node, don't change it
	if (isBoundaryNode(argIndex,coord)) return;
	if (loc==Location::BOUNDARY && useBoundaryDeterminationRule)
		insertBoundaryPoint(argIndex,coord);
	else
		insertPoint(argIndex,coord,loc);
}

vector<Edge*> *
GeometryGraph::getEdges()
{
	return edges;
}

bool
GeometryGraph::hasTooFewPoints()
{
	return hasTooFewPointsVar;
}

const Coordinate&
GeometryGraph::getInvalidPoint()
{
	return invalidPoint;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.23  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.22  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.21  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.20  2006/02/23 11:54:20  strk
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
 * Revision 1.19  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.18  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.17  2006/01/31 19:07:34  strk
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
 * Revision 1.16  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.15  2005/12/07 20:52:32  strk
 * minor container methods call reduction
 *
 * Revision 1.14  2005/11/16 22:21:45  strk
 * enforced const-correctness and use of initializer lists.
 *
 * Revision 1.13  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.12  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.11  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.10  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.9  2004/11/19 09:33:55  strk
 * removed useless CoordinateSequence copy in ::addLineString
 *
 * Revision 1.8  2004/11/05 11:41:57  strk
 * Made IsValidOp handle IllegalArgumentException throw from GeometryGraph
 * as a sign of invalidity (just for Polygon geometries).
 * Removed leaks generated by this specific exception.
 *
 * Revision 1.7  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.6  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.5  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.33  2003/11/12 15:43:38  strk
 * Added some more throw specifications
 *
 * Revision 1.32  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.31  2003/10/20 13:53:03  strk
 * LinearRing handled as a LineString in 
 * GeometryGraph::add(const Geometry *) - more explicit exception 
 * thrown for unknown geometries
 *
 * Revision 1.30  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 * Revision 1.29  2003/10/15 11:24:28  strk
 * Use getCoordinatesRO() introduced.
 *
 **********************************************************************/


