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
 **********************************************************************
 *
 * Last port: operation/valid/IsValidOp.java rev. 1.38
 *
 **********************************************************************/

#include <geos/opValid.h>
#include <typeinfo>
#include <set>
#include <geos/util.h>

namespace geos {

bool IsValidOp::isSelfTouchingRingFormingHoleValid = false;

/**
 * Find a point from the list of testCoords
 * that is NOT a node in the edge for the list of searchCoords
 *
 * @return the point found, or <code>null</code> if none found
 */
const Coordinate *
IsValidOp::findPtNotNode(const CoordinateSequence *testCoords,
	const LinearRing *searchRing, GeometryGraph *graph)
{
	// find edge corresponding to searchRing.
	Edge *searchEdge=graph->findEdge(searchRing);
	// find a point in the testCoords which is not a node of the searchRing
	EdgeIntersectionList &eiList=searchEdge->getEdgeIntersectionList();
	// somewhat inefficient - is there a better way? (Use a node map, for instance?)
	for(int i=0;i<testCoords->getSize(); i++) {
		const Coordinate& pt=testCoords->getAt(i);
		if (!eiList.isIntersection(pt)) {
			return &pt;
		}
	}
	return NULL;
}

IsValidOp::IsValidOp(const Geometry *geom):
	parentGeometry(geom),
	isChecked(false),
	validErr(NULL)
{
}

IsValidOp::~IsValidOp()
{
	delete validErr;
}

bool
IsValidOp::isValid()
{
	checkValid(parentGeometry);
	return validErr==NULL;
}

/*
 * Checks whether a coordinate is valid for processing.
 * Coordinates are valid iff their x and y coordinates are in the
 * range of the floating point representation.
 *
 * @param coord the coordinate to validate
 * @return <code>true</code> if the coordinate is valid
 */
bool
IsValidOp::isValid(const Coordinate &coord)
{
	if (! FINITE(coord.x) ) return false;
	if (! FINITE(coord.y) ) return false;
	return true;
}

TopologyValidationError *
IsValidOp::getValidationError()
{
	checkValid(parentGeometry);
	return validErr;
}

void
IsValidOp::checkValid(const Geometry *g)
{
	if (isChecked) return;
	validErr=NULL;

	// empty geometries are always valid!
	if (g->isEmpty()) return;

	const GeometryCollection *gc;

	if (typeid(*g)==typeid(Point)) checkValid((Point *)g);
	else if (typeid(*g)==typeid(LinearRing)) checkValid((LinearRing*)g);
	else if (typeid(*g)==typeid(LineString)) checkValid((LineString*)g);
	else if (typeid(*g)==typeid(Polygon)) checkValid((Polygon*)g);
	else if (typeid(*g)==typeid(MultiPolygon)) checkValid((MultiPolygon*)g);
	else if ((gc=dynamic_cast<const GeometryCollection *>(g)))
		checkValid(gc);
	else throw new UnsupportedOperationException();
}

/*
 * Checks validity of a Point.
 */
void
IsValidOp::checkValid(const Point *g)
{
	checkInvalidCoordinates(g->getCoordinatesRO());
}

/*
 * Checks validity of a LineString.  Almost anything goes for linestrings!
 */
void
IsValidOp::checkValid(const LineString *g)
{
	checkInvalidCoordinates(g->getCoordinatesRO());
	if (validErr != NULL) return;

	GeometryGraph graph(0,g);
	checkTooFewPoints(&graph);
}

/**
 * Checks validity of a LinearRing.
 */
void
IsValidOp::checkValid(const LinearRing *g){
	checkInvalidCoordinates(g->getCoordinatesRO());
	if (validErr != NULL) return;

	checkClosedRing(g);
	if (validErr != NULL) return;

	GeometryGraph graph(0, g);
	checkTooFewPoints(&graph);
	if (validErr!=NULL) return;

	LineIntersector li;
	delete graph.computeSelfNodes(&li, true);
	checkNoSelfIntersectingRings(&graph);
}

/**
 * Checks the validity of a polygon.
 * Sets the validErr flag.
 */
void
IsValidOp::checkValid(const Polygon *g)
{
	checkInvalidCoordinates(g);
	if (validErr != NULL) return;

	checkClosedRings(g);
	if (validErr != NULL) return;

	GeometryGraph graph(0,g);

	checkTooFewPoints(&graph);
	if (validErr!=NULL) return;

	checkConsistentArea(&graph);
	if (validErr!=NULL) return;

	if (!isSelfTouchingRingFormingHoleValid) {
		checkNoSelfIntersectingRings(&graph);
		if (validErr!=NULL) return;
	}

	checkHolesInShell(g,&graph);
	if (validErr!=NULL) return;

	checkHolesNotNested(g,&graph);
	if (validErr!=NULL) return;

	checkConnectedInteriors(graph);
}

void
IsValidOp::checkValid(const MultiPolygon *g)
{
	int ngeoms = g->getNumGeometries();
	vector<const Polygon *>polys(ngeoms);

	for (int i=0; i<ngeoms; ++i)
	{
		const Polygon *p = (const Polygon *)g->getGeometryN(i);

		checkInvalidCoordinates(p);
		if (validErr != NULL) return;

		checkClosedRings(p);
		if (validErr != NULL) return;

		polys[i]=p;
	}

	GeometryGraph graph(0,g);

	checkTooFewPoints(&graph);
	if (validErr!=NULL) return;

	checkConsistentArea(&graph);
	if (validErr!=NULL) return;

	if (!isSelfTouchingRingFormingHoleValid)
	{
		checkNoSelfIntersectingRings(&graph);
		if (validErr!=NULL) return;
	}

	for(int i=0; i<ngeoms; ++i)
	{
		const Polygon *p=polys[i]; 
		checkHolesInShell(p, &graph);
		if (validErr!=NULL) return;
	}

	for(int i=0; i<ngeoms; ++i)
	{
		const Polygon *p=polys[i];
		checkHolesNotNested(p, &graph);
		if (validErr!=NULL) return;
	}

	checkShellsNotNested(g,&graph);
	if (validErr!=NULL) return;

	checkConnectedInteriors(graph);
}

void
IsValidOp::checkValid(const GeometryCollection *gc)
{
	int ngeoms = gc->getNumGeometries();
	for(int i=0; i<ngeoms; ++i)
	{
		const Geometry *g=gc->getGeometryN(i);
		checkValid(g);
		if (validErr!=NULL) return;
	}
}

void
IsValidOp::checkTooFewPoints(GeometryGraph *graph)
{
	if (graph->hasTooFewPoints()) {
		validErr=new TopologyValidationError(
			TopologyValidationError::TOO_FEW_POINTS,
			graph->getInvalidPoint());
		return;
	}
}

/**
 * Checks that the arrangement of edges in a polygonal geometry graph
 * forms a consistent area.
 *
 * @param graph
 *
 * @see ConsistentAreaTester
 */
void
IsValidOp::checkConsistentArea(GeometryGraph *graph)
{
	ConsistentAreaTester cat(graph);
	bool isValidArea=cat.isNodeConsistentArea();

	if (!isValidArea)
	{
		validErr=new TopologyValidationError(
			TopologyValidationError::SELF_INTERSECTION,
			cat.getInvalidPoint());
		return;
	}

	if (cat.hasDuplicateRings())
	{
		validErr=new TopologyValidationError(
			TopologyValidationError::DUPLICATE_RINGS,
			cat.getInvalidPoint());
	}
}


/**
 * Check that there is no ring which self-intersects (except of course at its endpoints).
 * This is required by OGC topology rules (but not by other models
 * such as ESRI SDE, which allow inverted shells and exverted holes).
 *
 * @param graph the topology graph of the geometry
 */
void
IsValidOp::checkNoSelfIntersectingRings(GeometryGraph *graph)
{
	vector<Edge*> *edges=graph->getEdges();
	for(unsigned int i=0; i<edges->size(); ++i)
	{
		Edge *e=(*edges)[i];
		checkNoSelfIntersectingRing(e->getEdgeIntersectionList());
		if(validErr!=NULL) return;
	}
}

/**
 * check that a ring does not self-intersect, except at its endpoints.
 * Algorithm is to count the number of times each node along edge occurs.
 * If any occur more than once, that must be a self-intersection.
 */
void
IsValidOp::checkNoSelfIntersectingRing(EdgeIntersectionList &eiList)
{
	set<const Coordinate*,CoordLT>nodeSet;
	bool isFirst=true;
	EdgeIntersectionListIterator it=eiList.begin();
	EdgeIntersectionListIterator end=eiList.end();
	for(; it!=end; ++it)
	{
		EdgeIntersection *ei=*it;
		if (isFirst) {
			isFirst=false;
			continue;
		}
		if (nodeSet.find(&ei->coord)!=nodeSet.end()) {
			validErr=new TopologyValidationError(
				TopologyValidationError::RING_SELF_INTERSECTION,
				ei->coord);
			return;
		} else {
			nodeSet.insert(&ei->coord);
		}
	}
}

/**
 * Test that each hole is inside the polygon shell.
 * This routine assumes that the holes have previously been tested
 * to ensure that all vertices lie on the shell or inside it.
 * A simple test of a single point in the hole can be used,
 * provide the point is chosen such that it does not lie on the
 * boundary of the shell.
 *
 * @param p the polygon to be tested for hole inclusion
 * @param graph a GeometryGraph incorporating the polygon
 */
void
IsValidOp::checkHolesInShell(const Polygon *p,GeometryGraph *graph)
{
	LinearRing *shell=(LinearRing*) p->getExteriorRing();

	//SimplePointInRing pir(shell);
	//SIRtreePointInRing pir(shell);
	MCPointInRing pir(shell);

	int nholes = p->getNumInteriorRing();
	for(int i=0; i<nholes; ++i)
	{
		const LinearRing *hole=(const LinearRing*) p->getInteriorRingN(i);

		const Coordinate *holePt=findPtNotNode(hole->getCoordinatesRO(), shell, graph);

		/**
		 * If no non-node hole vertex can be found, the hole must
		 * split the polygon into disconnected interiors.
		 * This will be caught by a subsequent check.
		 */
		if (holePt==NULL) return;

		bool outside = !pir.isInside(*holePt);
		if (outside) {
			validErr=new TopologyValidationError(
				TopologyValidationError::HOLE_OUTSIDE_SHELL,
				*holePt);
			return;
		}
	}
}

/**
 * Tests that no hole is nested inside another hole.
 * This routine assumes that the holes are disjoint.
 * To ensure this, holes have previously been tested
 * to ensure that:
 * 
 *  - they do not partially overlap
 *    (checked by <code>checkRelateConsistency</code>)
 *  - they are not identical
 *    (checked by <code>checkRelateConsistency</code>)
 * 
 */
void
IsValidOp::checkHolesNotNested(const Polygon *p, GeometryGraph *graph)
{
	//SimpleNestedRingTester nestedTester(graph);
	//SweeplineNestedRingTester nestedTester(graph);
	QuadtreeNestedRingTester nestedTester(graph);

	int nholes=p->getNumInteriorRing();
	for(int i=0; i<nholes; ++i)
	{
		LinearRing *innerHole=(LinearRing*) p->getInteriorRingN(i);
		nestedTester.add(innerHole);
	}

	bool isNonNested=nestedTester.isNonNested();
	if (!isNonNested)
	{
		validErr=new TopologyValidationError(
			TopologyValidationError::NESTED_HOLES,
			*(nestedTester.getNestedPoint()));
	}
}

/*
 * Tests that no element polygon is wholly in the interior of another
 * element polygon.
 * 
 * Preconditions:
 * 
 * - shells do not partially overlap
 * - shells do not touch along an edge
 * - no duplicate rings exist
 *
 * This routine relies on the fact that while polygon shells may touch at
 * one or more vertices, they cannot touch at ALL vertices.
 */
void
IsValidOp::checkShellsNotNested(const MultiPolygon *mp, GeometryGraph *graph)
{
	int ngeoms = mp->getNumGeometries();
	for(int i=0; i<ngeoms; ++i)
	{
		const Polygon *p=(const Polygon *)mp->getGeometryN(i);
		const LinearRing *shell=(const LinearRing*) p->getExteriorRing();

		for(int j=0; j<ngeoms; ++j)
		{
			if (i==j) continue;
			const Polygon *p2=(const Polygon*) mp->getGeometryN(j);
			checkShellNotNested(shell, p2, graph);
			if (validErr!=NULL) return;
		}
	}
}

/**
 * Check if a shell is incorrectly nested within a polygon.  This is the case
 * if the shell is inside the polygon shell, but not inside a polygon hole.
 * (If the shell is inside a polygon hole, the nesting is valid.)
 * 
 * The algorithm used relies on the fact that the rings must be properly contained.
 * E.g. they cannot partially overlap (this has been previously checked by
 * <code>checkRelateConsistency</code>
 */
void
IsValidOp::checkShellNotNested(const LinearRing *shell, const Polygon *p,
	GeometryGraph *graph)
{
	const CoordinateSequence *shellPts=shell->getCoordinatesRO();

	// test if shell is inside polygon shell
	const LinearRing *polyShell=(const LinearRing*) p->getExteriorRing();
	const CoordinateSequence *polyPts=polyShell->getCoordinatesRO();
	const Coordinate *shellPt=findPtNotNode(shellPts,polyShell,graph);

	// if no point could be found, we can assume that the shell is outside the polygon
	if (shellPt==NULL) return;

	bool insidePolyShell=CGAlgorithms::isPointInRing(*shellPt, polyPts);
	if (!insidePolyShell) return;

	// if no holes, this is an error!
	int nholes = p->getNumInteriorRing();
	if (nholes<=0) {
		validErr=new TopologyValidationError(
			TopologyValidationError::NESTED_SHELLS,
			*shellPt);
		return;
	}
	
	/**
	 * Check if the shell is inside one of the holes.
	 * This is the case if one of the calls to checkShellInsideHole
	 * returns a null coordinate.
	 * Otherwise, the shell is not properly contained in a hole, which is
	 * an error.
	 */
	const Coordinate *badNestedPt=NULL;
	for(int i=0; i<nholes; ++i) {
		const LinearRing *hole=(const LinearRing*) p->getInteriorRingN(i);
		badNestedPt = checkShellInsideHole(shell, hole, graph);
		if (badNestedPt==NULL) return;
	}
	validErr=new TopologyValidationError(
		TopologyValidationError::NESTED_SHELLS, *badNestedPt
	);
}

/*
 * This routine checks to see if a shell is properly contained in a hole.
 * It assumes that the edges of the shell and hole do not
 * properly intersect.
 *
 * @return <code>null</code> if the shell is properly contained, or
 *   a Coordinate which is not inside the hole if it is not
 *
 */
const Coordinate *
IsValidOp::checkShellInsideHole(const LinearRing *shell, const LinearRing *hole,
	GeometryGraph *graph)
{
	const CoordinateSequence *shellPts=shell->getCoordinatesRO();
	const CoordinateSequence *holePts=hole->getCoordinatesRO();

	// TODO: improve performance of this - by sorting pointlists for instance?
	const Coordinate *shellPt=findPtNotNode(shellPts, hole, graph);
	// if point is on shell but not hole, check that the shell is inside the hole
	if (shellPt) {
		bool insideHole=CGAlgorithms::isPointInRing(*shellPt, holePts);
		if (!insideHole) return shellPt;
	}

	const Coordinate *holePt=findPtNotNode(holePts, shell, graph);
	// if point is on hole but not shell, check that the hole is outside the shell
	if (holePt) {
		bool insideShell=CGAlgorithms::isPointInRing(*holePt, shellPts);
		if (insideShell) return holePt;
		return NULL;
	}
	Assert::shouldNeverReachHere("points in shell and hole appear to be equal");
	return NULL;
}

void
IsValidOp::checkConnectedInteriors(GeometryGraph &graph)
{
	ConnectedInteriorTester cit(graph);
	if (!cit.isInteriorsConnected())
	{
		validErr=new TopologyValidationError(
			TopologyValidationError::DISCONNECTED_INTERIOR,
			cit.getCoordinate());
	}
}


void
IsValidOp::checkInvalidCoordinates(const CoordinateSequence *cs)
{
	int size=cs->getSize();
	for (int i=0; i<size; ++i)
	{
		if (! isValid(cs->getAt(i)) )
		{
			validErr = new TopologyValidationError(
				TopologyValidationError::INVALID_COORDINATE,
				cs->getAt(i));
			return;

		}
	}
}

void
IsValidOp::checkInvalidCoordinates(const Polygon *poly)
{
	checkInvalidCoordinates(poly->getExteriorRing()->getCoordinatesRO());
	if (validErr != NULL) return;

	int nholes=poly->getNumInteriorRing();
	for (int i=0; i<nholes; ++i)
	{
		checkInvalidCoordinates(
			poly->getInteriorRingN(i)->getCoordinatesRO()
		);
		if (validErr != NULL) return;
	}
}

void
IsValidOp::checkClosedRings(const Polygon *poly)
{
	const LinearRing *lr=(const LinearRing *)poly->getExteriorRing();
	checkClosedRing(lr);
	if (validErr) return;

	int nholes=poly->getNumInteriorRing();
	for (int i=0; i<nholes; ++i)
	{
		lr=(const LinearRing *)poly->getInteriorRingN(i);
		checkClosedRing(lr);
		if (validErr) return;
	}
}

void
IsValidOp::checkClosedRing(const LinearRing *ring)
{
	if ( ! ring->isClosed() )
	{
		validErr = new TopologyValidationError(
			TopologyValidationError::RING_NOT_CLOSED,
				ring->getCoordinateN(0));
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.36  2005/11/21 16:03:20  strk
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
 * Revision 1.35  2005/11/16 22:21:45  strk
 * enforced const-correctness and use of initializer lists.
 *
 * Revision 1.34  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.33  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.32  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.31  2005/11/01 09:40:42  strk
 * Replaced finite() with FINITE() calls.
 *
 * Revision 1.30  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.29  2005/04/07 18:43:20  strk
 * Fixed bug throwing an exception when a result could be returned instead
 * (ported JTS patch)
 *
 * Revision 1.28  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.27  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.26  2004/11/06 08:16:46  strk
 * Fixed CGAlgorithms::isCCW from JTS port.
 * Code cleanup in IsValidOp.
 *
 * Revision 1.25  2004/11/05 11:41:57  strk
 * Made IsValidOp handle IllegalArgumentException throw from GeometryGraph
 * as a sign of invalidity (just for Polygon geometries).
 * Removed leaks generated by this specific exception.
 *
 * Revision 1.24  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.23  2004/09/13 12:50:11  strk
 * comments cleanup
 *
 * Revision 1.22  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.21  2004/09/13 10:12:49  strk
 * Added invalid coordinates checks in IsValidOp.
 * Cleanups.
 *
 * Revision 1.20  2004/09/13 09:18:10  strk
 * Added IsValidOp::isValid(Coordinate &)
 *
 * Revision 1.19  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.18  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.17  2004/05/18 00:02:37  ybychkov
 * IsValidOp::checkShellNotNested() bugfix from JTS 1.4.1 (not released yet) has been added.
 *
 * Revision 1.16  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.15  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.14  2003/10/15 11:24:28  strk
 * Use getCoordinatesRO() introduced.
 *
 * Revision 1.13  2003/10/13 17:54:40  strk
 * IsValidOp constructor used same name for the arg and a private 
 * element. Fixed.
 *
 **********************************************************************/

