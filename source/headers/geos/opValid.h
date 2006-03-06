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
namespace operation { // geos.operation

/// Provides classes for testing the validity of geometries.
namespace valid { // geos.operation.valid

/**
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using a simple O(n^2)
 * comparison.
 *
 */
class SimpleNestedRingTester {
private:
	geomgraph::GeometryGraph *graph;  // used to find non-node vertices
	std::vector<LinearRing*> rings;
	Coordinate *nestedPt;
public:
	SimpleNestedRingTester(geomgraph::GeometryGraph *newGraph)
		:
		graph(newGraph),
		rings(),
		nestedPt(NULL)
	{}

	~SimpleNestedRingTester() {
	}

	void add(LinearRing *ring) {
		rings.push_back(ring);
	}

	/*
	 * Be aware that the returned Coordinate (if != NULL)
	 * will point to storage owned by one of the LinearRing
	 * previously added. If you destroy them, this
	 * will point to an invalid memory address.
	 */
	Coordinate *getNestedPoint() {
		return nestedPt;
	}

	bool isNonNested();
};

/**
 * Contains information about the nature and location of a {@link Geometry}
 * validation error
 *
 */
class TopologyValidationError {
public:

	enum errorEnum {
		eError,
		eRepeatedPoint,
		eHoleOutsideShell,
		eNestedHoles,
		eDisconnectedInterior,
		eSelfIntersection,
		eRingSelfIntersection,
		eNestedShells,
		eDuplicatedRings,
		eTooFewPoints,
		eInvalidCoordinate,
		eRingNotClosed
	};

	TopologyValidationError(int newErrorType, const Coordinate& newPt);
	TopologyValidationError(int newErrorType);
	Coordinate& getCoordinate();
	std::string getMessage();
	int getErrorType();
	std::string toString();

private:
	// Used const char* to reduce dynamic allocations
	static const char* errMsg[];
	int errorType;
	Coordinate pt;
};

/**
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

/**
 * Checks that a {@link geomgraph::GeometryGraph} representing an area
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

	algorithm::LineIntersector *li;

	geomgraph::GeometryGraph *geomGraph;

	relate::RelateNodeGraph *nodeGraph;

	/// the intersection point found (if any)
	Coordinate invalidPoint;

	/**
	 * Check all nodes to see if their labels are consistent.
	 * If any are not, return false
	 */
	bool isNodeEdgeAreaLabelsConsistent();

public:

	ConsistentAreaTester(geomgraph::GeometryGraph *newGeomGraph);

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
	 * more than one geomgraph::EdgeEnd.
	 * (This is because topologically consistent areas cannot have two rings sharing
	 * the same line segment, unless the rings are equal).
	 * The start point of one of the equal rings will be placed in
	 * invalidPoint.
	 *
	 * @return true if this area Geometry is topologically consistent but has two duplicate rings
	 */
	bool hasDuplicateRings();
};


/**
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using an
 * index::sweepline::SweepLineIndex to speed up the comparisons.
 *
 */
class SweeplineNestedRingTester {

private:
	geomgraph::GeometryGraph *graph;  // used to find non-node vertices
	std::vector<LinearRing*> rings;
	Envelope *totalEnv;
	index::sweepline::SweepLineIndex *sweepLine;
	Coordinate *nestedPt;
	void buildIndex();

public:

	SweeplineNestedRingTester(geomgraph::GeometryGraph *newGraph)
		:
		graph(newGraph),
		rings(),
		totalEnv(new Envelope()),
		sweepLine(new index::sweepline::SweepLineIndex()),
		nestedPt(NULL)
	{}

	~SweeplineNestedRingTester()
	{
		delete totalEnv;
		delete sweepLine;
	}

	/*
	 * Be aware that the returned Coordinate (if != NULL)
	 * will point to storage owned by one of the LinearRing
	 * previously added. If you destroy them, this
	 * will point to an invalid memory address.
	 */
	Coordinate *getNestedPoint() { return nestedPt; }

	void add(LinearRing* ring) {
		rings.push_back(ring);
	}

	bool isNonNested();
	bool isInside(LinearRing *innerRing,LinearRing *searchRing);
	class OverlapAction: public index::sweepline::SweepLineOverlapAction {
	public:
		bool isNonNested;
		OverlapAction(SweeplineNestedRingTester *p);
		void overlap(index::sweepline::SweepLineInterval *s0,
				index::sweepline::SweepLineInterval *s1);
	private:
		SweeplineNestedRingTester *parent;
	};
};

/*
 * Tests whether any of a set of {@link LinearRing}s are
 * nested inside another ring in the set, using a {@link Quadtree}
 * index to speed up the comparisons.
 *
 */
class QuadtreeNestedRingTester {
public:
	QuadtreeNestedRingTester(geomgraph::GeometryGraph *newGraph);
	virtual ~QuadtreeNestedRingTester();
	/*
	 * Be aware that the returned Coordinate (if != NULL)
	 * will point to storage owned by one of the LinearRing
	 * previously added. If you destroy them, this
	 * will point to an invalid memory address.
	 */
	Coordinate *getNestedPoint();
	void add(LinearRing *ring);
	bool isNonNested();
private:
	geomgraph::GeometryGraph *graph;  // used to find non-node vertices
	std::vector<LinearRing*> *rings;
	Envelope *totalEnv;
	index::quadtree::Quadtree *qt;
	Coordinate *nestedPt;
	void buildQuadtree();
};

/*
 * This class tests that the interior of an area Geometry
 * (Polygon or MultiPolygon)
 * is connected.  An area Geometry is invalid if the interior is disconnected.
 * This can happen if:
 * 
 * - one or more holes either form a chain touching the shell at two places
 * - one or more holes form a ring around a portion of the interior
 * 
 * If an inconsistency if found the location of the problem
 * is recorded.
 */
class ConnectedInteriorTester {
public:
	ConnectedInteriorTester(geomgraph::GeometryGraph &newGeomGraph);
	~ConnectedInteriorTester();
	Coordinate& getCoordinate();
	bool isInteriorsConnected();
	static const Coordinate& findDifferentPoint(const CoordinateSequence *coord, const Coordinate& pt);

private:

	GeometryFactory *geometryFactory;

	geomgraph::GeometryGraph &geomGraph;

	/// Save a coordinate for any disconnected interior found
	/// the coordinate will be somewhere on the ring surrounding
	/// the disconnected interior
	Coordinate disconnectedRingcoord;

	void setAllEdgesInResult(geomgraph::PlanarGraph &graph);

	std::vector<geomgraph::EdgeRing*>* buildEdgeRings(
			std::vector<geomgraph::EdgeEnd*> *dirEdges);

	/**
	 * Mark all the edges for the edgeRings corresponding to the shells
	 * of the input polygons.  Note only ONE ring gets marked for each shell.
	 */
	void visitShellInteriors(const Geometry *g, geomgraph::PlanarGraph &graph);

	void visitInteriorRing(const LineString *ring, geomgraph::PlanarGraph &graph);

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
	bool hasUnvisitedShellEdge(std::vector<geomgraph::EdgeRing*> *edgeRings);

protected:

	void visitLinkedDirectedEdges(geomgraph::DirectedEdge *start);
};

/*
 * Implements the algorithsm required to compute the <code>isValid()</code> method
 * for {@link Geometry}s.
 *
 */
class IsValidOp {
friend class Unload;
private:
	const Geometry *parentGeometry;  // the base Geometry to be validated
	bool isChecked;
	TopologyValidationError* validErr;
	void checkValid(const Geometry *g);
	void checkValid(const Point *g);
	void checkValid(const LinearRing *g);
	void checkValid(const LineString *g);
	void checkValid(const Polygon *g);
	void checkValid(const MultiPolygon *g);
	void checkValid(const GeometryCollection *gc);
	void checkConsistentArea(geomgraph::GeometryGraph *graph);
	void checkNoSelfIntersectingRings(geomgraph::GeometryGraph *graph);

	/**
	 * check that a ring does not self-intersect, except at its endpoints.
	 * Algorithm is to count the number of times each node along edge occurs.
	 * If any occur more than once, that must be a self-intersection.
	 */
	void checkNoSelfIntersectingRing(geomgraph::EdgeIntersectionList &eiList);

	void checkTooFewPoints(geomgraph::GeometryGraph *graph);

	/**
	 * Test that each hole is inside the polygon shell.
	 * This routine assumes that the holes have previously been tested
	 * to ensure that all vertices lie on the shell or inside it.
	 * A simple test of a single point in the hole can be used,
	 * provide the point is chosen such that it does not lie on the
	 * boundary of the shell.
	 *
	 * @param p the polygon to be tested for hole inclusion
	 * @param graph a geomgraph::GeometryGraph incorporating the polygon
	 */
	void checkHolesInShell(const Polygon *p, geomgraph::GeometryGraph *graph);

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
	void checkHolesNotNested(const Polygon *p, geomgraph::GeometryGraph *graph);

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
	void checkShellsNotNested(const MultiPolygon *mp, geomgraph::GeometryGraph *graph);

	/**
	 * Check if a shell is incorrectly nested within a polygon.  This is the case
	 * if the shell is inside the polygon shell, but not inside a polygon hole.
	 * (If the shell is inside a polygon hole, the nesting is valid.)
	 * 
	 * The algorithm used relies on the fact that the rings must be properly contained.
	 * E.g. they cannot partially overlap (this has been previously checked by
	 * <code>checkRelateConsistency</code>
	 */
	void checkShellNotNested(const LinearRing *shell, const Polygon *p, geomgraph::GeometryGraph *graph);

	/**
	 * This routine checks to see if a shell is properly contained in a hole.
	 */
	const Coordinate *checkShellInsideHole(const LinearRing *shell, const LinearRing *hole, geomgraph::GeometryGraph *graph);

	void checkConnectedInteriors(geomgraph::GeometryGraph &graph);

	void checkInvalidCoordinates(const CoordinateSequence *cs);

	void checkInvalidCoordinates(const Polygon *poly);

	void checkClosedRings(const Polygon *poly);

	void checkClosedRing(const LinearRing *ring);

	bool isSelfTouchingRingFormingHoleValid;

public:
	/**
	 * Find a point from the list of testCoords
	 * that is NOT a node in the edge for the list of searchCoords
	 *
	 * @return the point found, or NULL if none found
	 */
	static const Coordinate *findPtNotNode(
			const CoordinateSequence *testCoords,
			const LinearRing *searchRing, geomgraph::GeometryGraph *graph);

	/**
	 * Checks whether a coordinate is valid for processing.
	 * Coordinates are valid iff their x and y coordinates are in the
	 * range of the floating point representation.
	 *
	 * @param coord the coordinate to validate
	 * @return <code>true</code> if the coordinate is valid
	 */
	static bool isValid(const Coordinate &coord);

	IsValidOp(const Geometry *geom)
		:
		parentGeometry(geom),
		isChecked(false),
		validErr(NULL),
		isSelfTouchingRingFormingHoleValid(false)
	{}

	/// TODO: validErr can't be a pointer!
	virtual ~IsValidOp() {
		delete validErr;
	}

	bool isValid();
	TopologyValidationError* getValidationError();

	/**
	 * Sets whether polygons using <b>Self-Touching Rings</b> to form
	 * holes are reported as valid.
	 * If this flag is set, the following Self-Touching conditions
	 * are treated as being valid:
	 * <ul>
	 * <li>the shell ring self-touches to create a hole touching the shell
	 * <li>a hole ring self-touches to create two holes touching at a point
	 * </ul>
	 * <p>
	 * The default (following the OGC SFS standard)
	 * is that this condition is <b>not</b> valid (<code>false</code>).
	 * <p>
	 * This does not affect whether Self-Touching Rings
	 * disconnecting the polygon interior are considered valid
	 * (these are considered to be <b>invalid</b> under the SFS,
	 * and many other spatial models as well).
	 * This includes "bow-tie" shells,
	 * which self-touch at a single point causing the interior to
	 * be disconnected,
	 * and "C-shaped" holes which self-touch at a single point causing
	 * an island to be formed.
	 *
	 * @param isValid states whether geometry with this condition is valid
	 */
	void setSelfTouchingRingFormingHoleValid(bool isValid)
	{
		isSelfTouchingRingFormingHoleValid = isValid;
	}

};

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/03/06 12:47:49  strk
 * TopologyValidationError error names (enum) renamed to avoid conflicts.
 *
 * Revision 1.16  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.15  2006/03/01 10:48:54  strk
 * Changed static TopologyValidationError::errMsg[] from 'string' to 'const char*'
 * to reduce dynamic memory allocations.
 *
 * Revision 1.14  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.13  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.12  2006/02/08 17:18:28  strk
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
 * Revision 1.11  2005/11/21 16:03:20  strk
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
 * Revision 1.10  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.9  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.8  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.7  2004/11/05 11:41:57  strk
 * Made IsValidOp handle IllegalArgumentException throw from GeometryGraph
 * as a sign of invalidity (just for Polygon geometries).
 * Removed leaks generated by this specific exception.
 *
 * Revision 1.6  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.5  2004/09/13 10:12:49  strk
 * Added invalid coordinates checks in IsValidOp.
 * Cleanups.
 *
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

