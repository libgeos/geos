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
 **********************************************************************
 * $Log$
 * Revision 1.4  2004/09/13 09:18:10  strk
 * Added IsValidOp::isValid(Coordinate &)
 *
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.15  2004/05/18 00:02:37  ybychkov
 * IsValidOp::checkShellNotNested() bugfix from JTS 1.4.1 (not released yet) has been added.
 *
 * Revision 1.14  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_OPVALID_H
#define GEOS_OPVALID_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <geos/platform.h>
#include <geos/opRelate.h>
#include <geos/indexSweepline.h>
#include <geos/indexQuadtree.h>

namespace geos {

/*
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using a simple O(n^2)
 * comparison.
 *
 */
class SimpleNestedRingTester {
public:
	SimpleNestedRingTester(GeometryGraph *newGraph);
	~SimpleNestedRingTester();
	void add(LinearRing *ring);
	Coordinate& getNestedPoint();
	bool isNonNested();
private:
	CGAlgorithms *cga;
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Coordinate nestedPt;
};

/*
 * Contains information about the nature and location of a {@link Geometry}
 * validation error
 *
 */
class TopologyValidationError {
public:
	enum {
		ERROR,
		REPEATED_POINT,
		HOLE_OUTSIDE_SHELL,
		NESTED_HOLES,
		DISCONNECTED_INTERIOR,
		SELF_INTERSECTION,
		RING_SELF_INTERSECTION,
		NESTED_SHELLS,
		DUPLICATE_RINGS,
		TOO_FEW_POINTS
	};

TopologyValidationError(int newErrorType,Coordinate newPt);
TopologyValidationError(int newErrorType);
Coordinate& getCoordinate();
string getMessage();
int getErrorType();
string toString();
private:
	static string errMsg[];
	int errorType;
	Coordinate pt;
};

/*
 * Implements the appropriate checks for repeated points
 * (consecutive identical coordinates) as defined in the
 * JTS spec.
 */
class RepeatedPointTester {
public:
	RepeatedPointTester() {};
	Coordinate& getCoordinate();
	bool hasRepeatedPoint(const Geometry *g);
	bool hasRepeatedPoint(const CoordinateSequence *coord);
private:
	Coordinate repeatedCoord;
	bool hasRepeatedPoint(const Polygon *p);
	bool hasRepeatedPoint(const GeometryCollection *gc);
	bool hasRepeatedPoint(const MultiPolygon *gc);
	bool hasRepeatedPoint(const MultiLineString *gc);
};

/*
 * Checks that a {@link GeometryGraph} representing an area
 * (a {@link Polygon} or {@link MultiPolygon} )
 * is consistent with the SFS semantics for area geometries.
 * Checks include:
 * <ul>
 * <li>testing for rings which self-intersect (both properly
 * and at nodes)
 * <li>testing for duplicate rings
 * </ul>
 * If an inconsistency if found the location of the problem
 * is recorded.
 */
class ConsistentAreaTester {
private:
	LineIntersector *li;
	GeometryGraph *geomGraph;
	RelateNodeGraph *nodeGraph;
	// the intersection point found (if any)
	Coordinate invalidPoint;
	/**
	* Check all nodes to see if their labels are consistent.
	* If any are not, return false
	*/
	bool isNodeEdgeAreaLabelsConsistent();
public:
	ConsistentAreaTester(GeometryGraph *newGeomGraph);
	~ConsistentAreaTester();
	/**
	* @return the intersection point, or <code>null</code> if none was found
	*/
	Coordinate& getInvalidPoint();
	bool isNodeConsistentArea();
	/**
	* Checks for two duplicate rings in an area.
	* Duplicate rings are rings that are topologically equal
	* (that is, which have the same sequence of points up to point order).
	* If the area is topologically consistent (determined by calling the
	* <code>isNodeConsistentArea</code>,
	* duplicate rings can be found by checking for EdgeBundles which contain
	* more than one EdgeEnd.
	* (This is because topologically consistent areas cannot have two rings sharing
	* the same line segment, unless the rings are equal).
	* The start point of one of the equal rings will be placed in
	* invalidPoint.
	*
	* @return true if this area Geometry is topologically consistent but has two duplicate rings
	*/
	bool hasDuplicateRings();
};


/*
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using a {@link SweepLineIndex}
 * index to speed up the comparisons.
 *
 */
class SweeplineNestedRingTester {
public:
	SweeplineNestedRingTester(GeometryGraph *newGraph);
	~SweeplineNestedRingTester();
	Coordinate& getNestedPoint();
	void add(LinearRing* ring);
	bool isNonNested();
	bool isInside(LinearRing *innerRing,LinearRing *searchRing);
	class OverlapAction: public SweepLineOverlapAction {
	public:
		bool isNonNested;
		OverlapAction(SweeplineNestedRingTester *p);
		void overlap(SweepLineInterval *s0, SweepLineInterval *s1);
	private:
		SweeplineNestedRingTester *parent;
	};
private:
	CGAlgorithms *cga;
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Envelope *totalEnv;
	SweepLineIndex *sweepLine;
	Coordinate nestedPt;
	void buildIndex();
};

/*
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using a {@link Quadtree}
 * index to speed up the comparisons.
 *
 */
class QuadtreeNestedRingTester {
public:
	QuadtreeNestedRingTester(GeometryGraph *newGraph);
	virtual ~QuadtreeNestedRingTester();
	Coordinate& getNestedPoint();
	void add(LinearRing *ring);
	bool isNonNested();
private:
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Envelope *totalEnv;
	Quadtree *qt;
	Coordinate nestedPt;
	void buildQuadtree();
};

/*
 * This class tests that the interior of an area {@link Geometry}
 *  ({@link Polygon}  or {@link MultiPolygon} )
 * is connected.  An area Geometry is invalid if the interior is disconnected.
 * This can happen if:
 * <ul>
 * <li>one or more holes either form a chain touching the shell at two places
 * <li>one or more holes form a ring around a portion of the interior
 * </ul>
 * If an inconsistency if found the location of the problem
 * is recorded.
 */
class ConnectedInteriorTester {
public:
	ConnectedInteriorTester(GeometryGraph *newGeomGraph);
	~ConnectedInteriorTester();
	Coordinate& getCoordinate();
	bool isInteriorsConnected();
	static const Coordinate& findDifferentPoint(const CoordinateSequence *coord, const Coordinate& pt);
private:
	GeometryFactory *geometryFactory;
	CGAlgorithms *cga;
	GeometryGraph *geomGraph;
	// save a coordinate for any disconnected interior found
	// the coordinate will be somewhere on the ring surrounding the disconnected interior
	Coordinate disconnectedRingcoord;
	void setAllEdgesInResult(PlanarGraph *graph);
	vector<EdgeRing*>* buildEdgeRings(vector<EdgeEnd*> *dirEdges);
	/**
	* Mark all the edges for the edgeRings corresponding to the shells
	* of the input polygons.  Note only ONE ring gets marked for each shell.
	*/
	void visitShellInteriors(const Geometry *g, PlanarGraph *graph);
	void visitInteriorRing(const LineString *ring, PlanarGraph *graph);
	/**
	* Check if any shell ring has an unvisited edge.
	* A shell ring is a ring which is not a hole and which has the interior
	* of the parent area on the RHS.
	* (Note that there may be non-hole rings with the interior on the LHS,
	* since the interior of holes will also be polygonized into CW rings
	* by the linkAllDirectedEdges() step)
	*
	* @return true if there is an unvisited edge in a non-hole ring
	*/
	bool hasUnvisitedShellEdge(vector<EdgeRing*> *edgeRings);
protected:
	void visitLinkedDirectedEdges(DirectedEdge *start);
};

/*
 * Implements the algorithsm required to compute the <code>isValid()</code> method
 * for {@link Geometry}s.
 *
 */
class IsValidOp {
friend class Unload;
public:
	/**
	* Find a point from the list of testCoords
	* that is NOT a node in the edge for the list of searchCoords
	*
	* @return the point found, or <code>null</code> if none found
	*/
	static const Coordinate& findPtNotNode(const CoordinateSequence *testCoords,const LinearRing *searchRing, GeometryGraph *graph);

	/**
	 * Checks whether a coordinate is valid for processing.
	 * Coordinates are valid iff their x and y coordinates are in the
	 * range of the floating point representation.
	 *
	 * @param coord the coordinate to validate
	 * @return <code>true</code> if the coordinate is valid
	 */
	static bool isValid(const Coordinate &coord);

	IsValidOp(const Geometry *geom);
	virtual ~IsValidOp();
	bool isValid();
	TopologyValidationError* getValidationError();
private:
	const Geometry *parentGeometry;  // the base Geometry to be validated
	bool isChecked;
	TopologyValidationError* validErr;
	void checkValid(const Geometry *g);
	void checkValid(const LinearRing *g);
	void checkValid(const LineString *g);
	void checkValid(const Polygon *g);
	void checkValid(const MultiPolygon *g);
	void checkValid(const GeometryCollection *gc);
	void checkConsistentArea(GeometryGraph *graph);
	void checkNoSelfIntersectingRings(GeometryGraph *graph);
	/**
	* check that a ring does not self-intersect, except at its endpoints.
	* Algorithm is to count the number of times each node along edge occurs.
	* If any occur more than once, that must be a self-intersection.
	*/
	void checkSelfIntersectingRing(EdgeIntersectionList *eiList);
	void checkTooFewPoints(GeometryGraph *graph);
	/**
	* Test that each hole is inside the polygon shell.
	* This routine assumes that the holes have previously been tested
	* to ensure that all vertices lie on the shell or inside it.
	* A simple test of a single point in the hole can be used,
	* provide the point is chosen such that it does not lie on the
	* boundary of the shell.
	*/
	void checkHolesInShell(const Polygon *p,GeometryGraph *graph);
//	void OLDcheckHolesInShell(Polygon *p);
	/**
	* Tests that no hole is nested inside another hole.
	* This routine assumes that the holes are disjoint.
	* To ensure this, holes have previously been tested
	* to ensure that:
	* <ul>
	* <li>they do not partially overlap
	* (checked by <code>checkRelateConsistency</code>)
	* <li>they are not identical
	* (checked by <code>checkRelateConsistency</code>)
	* </ul>
	*/
	void checkHolesNotNested(const Polygon *p,GeometryGraph *graph);
//	void SLOWcheckHolesNotNested(Polygon *p);
	/**
	* Test that no element polygon is wholly in the interior of another element polygon.
	* TODO: It handles the case that one polygon is nested inside a hole of another.
	* <p>
	* Preconditions:
	* <ul>
	* <li>shells do not partially overlap
	* <li>shells do not touch along an edge
	* <li>no duplicate rings exist
	* </ul>
	* This routine relies on the fact that while polygon shells may touch at one or
	* more vertices, they cannot touch at ALL vertices.
	*/
	void checkShellsNotNested(const MultiPolygon *mp,GeometryGraph *graph);
	/**
	* Check if a shell is incorrectly nested within a polygon.  This is the case
	* if the shell is inside the polygon shell, but not inside a polygon hole.
	* (If the shell is inside a polygon hole, the nesting is valid.)
	* <p>
	* The algorithm used relies on the fact that the rings must be properly contained.
	* E.g. they cannot partially overlap (this has been previously checked by
	* <code>checkRelateConsistency</code>
	*/
	void checkShellNotNested(const LinearRing *shell,const Polygon *p,GeometryGraph *graph);
	/**
	* This routine checks to see if a shell is properly contained in a hole.
	*/
	const Coordinate& checkShellInsideHole(const LinearRing *shell,const LinearRing *hole,GeometryGraph *graph);
	void checkConnectedInteriors(GeometryGraph *graph);
};
}
#endif
