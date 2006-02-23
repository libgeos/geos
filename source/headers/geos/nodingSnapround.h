/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SNAPROUND_H
#define GEOS_NODING_SNAPROUND_H

#include <geos/platform.h>
#include <geos/noding.h>
#include <geos/geom.h>
#include <cmath>
#include <vector>

using namespace std;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

/**
 * Implements a "hot pixel" as used in the Snap Rounding algorithm.
 * A hot pixel contains the interior of the tolerance square and
 * the boundary
 * <b>minus</b> the top and right segments.
 * <p>
 * The hot pixel operations are all computed in the integer domain
 * to avoid rounding problems.
 *
 * @version 1.7
 */
class HotPixel {
private:
	algorithm::LineIntersector& li;

	Coordinate pt;
	const Coordinate& originalPt;
	Coordinate ptScaled;

	double scaleFactor;

	double minx;
	double maxx;
	double miny;
	double maxy;

	/**
	 * The corners of the hot pixel, in the order:
	 *  10
	 *  23
	 */
	vector<Coordinate> corner;

	// Owned by this class, constructed on demand
	mutable auto_ptr<Envelope> safeEnv; 

	void initCorners(const Coordinate& pt);

	double scale(double val) const {
		// Math.round
		return round(val*scaleFactor);
	}

	void copyScaled(const Coordinate& p, Coordinate& pScaled) const {
		pScaled.x = scale(p.x);
		pScaled.y = scale(p.y);
	}

	/**
	 * Tests whether the segment p0-p1 intersects the hot pixel
	 * tolerance square.
	 * Because the tolerance square point set is partially open (along the
	 * top and right) the test needs to be more sophisticated than
	 * simply checking for any intersection.  However, it
	 * can take advantage of the fact that because the hot pixel edges
	 * do not lie on the coordinate grid.  It is sufficient to check
	 * if there is at least one of:
	 * <ul>
	 * <li>a proper intersection with the segment and any hot pixel edge
	 * <li>an intersection between the segment and both the left
	 *     and bottom edges
	 * <li>an intersection between a segment endpoint and the hot
	 *     pixel coordinate
	 * </ul>
	 *
	 * @param p0
	 * @param p1
	 * @return
	 */
	bool intersectsToleranceSquare(const Coordinate& p0,
			const Coordinate& p1) const;
 

	/**
	 * Test whether the given segment intersects
	 * the closure of this hot pixel.
	 * This is NOT the test used in the standard snap-rounding
	 * algorithm, which uses the partially closed tolerance square
	 * instead.
	 * This routine is provided for testing purposes only.
	 *
	 * @param p0 the start point of a line segment
	 * @param p1 the end point of a line segment
	 * @return <code>true</code> if the segment intersects the
	 *         closure of the pixel's tolerance square
	 */
	bool intersectsPixelClosure(const Coordinate& p0,
			const Coordinate& p1);
 
public:

	HotPixel(const Coordinate& pt, double scaleFact, algorithm::LineIntersector& li);

	/// \brief
	/// Return reference to original Coordinate
	/// (the one provided at construction time)
	const Coordinate& getCoordinate() const { return originalPt; }

	/**
	 * Returns a "safe" envelope that is guaranteed to contain
	 * the hot pixel. Keeps ownership of it.
	 */
	const Envelope& getSafeEnvelope() const;

	bool intersectsScaled(const Coordinate& p0,
			const Coordinate& p1) const;

	bool intersects(const Coordinate& p0,
			const Coordinate& p1) const;
 

};


/**
 * Uses Snap Rounding to compute a rounded,
 * fully noded arrangement from a set of {@link SegmentString}s.
 * Implements the Snap Rounding technique described in
 * Hobby, Guibas & Marimont, and Goodrich et al.
 * Snap Rounding assumes that all vertices lie on a uniform grid
 * (hence the precision model of the input must be fixed precision,
 * and all the input vertices must be rounded to that precision).
 *
 * This implementation uses simple iteration over the line segments.
 *
 * This implementation appears to be fully robust using an integer
 * precision model.
 * It will function with non-integer precision models, but the
 * results are not 100% guaranteed to be correctly noded.
 *
 * Last port: noding/snapround/SimpleSnapRounder.java rev. 1.2 (JTS-1.7)
 *
 * TODO: remove Noder inheritance (that's an interface)
 * TODO: finish me
 *
 */
class SimpleSnapRounder: public Noder { // implements NoderIface

private:
	const PrecisionModel& pm;
	algorithm::LineIntersector li;
	double scaleFactor;
	SegmentString::NonConstVect* nodedSegStrings;

	void checkCorrectness(SegmentString::NonConstVect& inputSegmentStrings);

	void snapRound(SegmentString::NonConstVect* segStrings,
			algorithm::LineIntersector& li);

	/**
	 * Computes all interior intersections in the vector
	 * of SegmentString, and fill the given vector
	 * with their Coordinates.
	 *
	 * Does NOT node the segStrings.
	 *
	 * @param segStrings a vector of const Coordinates for the intersections
	 * @param li the algorithm::LineIntersector to use
	 * @param ret the vector to push intersection Coordinates to
	 */
	void findInteriorIntersections(SegmentString::NonConstVect& segStrings,
			algorithm::LineIntersector& li, vector<Coordinate>& ret);

	/**
	 * Computes nodes introduced as a result of snapping segments to snap points (hot pixels)
	 * @param li
	 */
	void computeSnaps(const SegmentString::NonConstVect& segStrings,
			vector<Coordinate>& snapPts);

	void computeSnaps(SegmentString* ss, vector<Coordinate>& snapPts);

	/**
	 * Performs a brute-force comparison of every segment in each {@link SegmentString}.
	 * This has n^2 performance.
	 */
	void computeVertexSnaps(SegmentString* e0, SegmentString* e1);

public:

	SimpleSnapRounder(const PrecisionModel& newPm):
		pm(newPm),
		li(&newPm),
		scaleFactor(newPm.getScale())
	{
	}

	SegmentString::NonConstVect* getNodedSubstrings() const {
		return nodedSegStrings;
	}

	void computeNodes(SegmentString::NonConstVect* inputSegmentStrings)
	{
		nodedSegStrings = inputSegmentStrings;
		snapRound(inputSegmentStrings, li);

		// testing purposes only - remove in final version
		checkCorrectness(*inputSegmentStrings);
	}

	void add(const SegmentString* segStr);

	/**
	 * Adds a new node (equal to the snap pt) to the segment if the segment
	 * passes through the hot pixel.
	 *
	 * @param hotPix
	 * @param segStr
	 * @param segIndex
	 * @return <code>true</code> if a node was added
	 */
	static bool addSnappedNode(const HotPixel& hotPix, SegmentString& segStr,
			unsigned int segIndex);

	/**
	 * Computes nodes introduced as a result of
	 * snapping segments to vertices of other segments
	 *
	 * @param segStrings the list of segment strings to snap together
	 */
	void computeVertexSnaps(const SegmentString::NonConstVect& edges);
};

/**
 * "Snaps" all {@link SegmentString}s in a {@link SpatialIndex} containing
 * {@link MonotoneChain}s to a given {@link HotPixel}.
 *
 * Last port: noding/snapround/MCIndexPointSnapper.java rev. 1.2 (JTS-1.7)
 */
class MCIndexPointSnapper {

private:

	index::SpatialIndex& index;

public:
 

	MCIndexPointSnapper(index::SpatialIndex& nIndex)
		:
		index(nIndex)
	{}


	/**
	 * Snaps (nodes) all interacting segments to this hot pixel.
	 * The hot pixel may represent a vertex of an edge,
	 * in which case this routine uses the optimization
	 * of not noding the vertex itself
	 *
	 * @param hotPixel the hot pixel to snap to
	 * @param parentEdge the edge containing the vertex,
	 *        if applicable, or <code>null</code>
	 * @param vertexIndex the index of the vertex, if applicable, or -1
	 * @return <code>true</code> if a node was added for this pixel
	 */
	bool snap(const HotPixel& hotPixel, SegmentString* parentEdge,
			unsigned int vertexIndex);

	bool snap(const HotPixel& hotPixel) {
		return snap(hotPixel, NULL, 0);
	}
		
};

/**
 * Uses Snap Rounding to compute a rounded,
 * fully noded arrangement from a set of {@link SegmentString}s.
 * Implements the Snap Rounding technique described in Hobby, Guibas & Marimont,
 * and Goodrich et al.
 * Snap Rounding assumes that all vertices lie on a uniform grid
 * (hence the precision model of the input must be fixed precision,
 * and all the input vertices must be rounded to that precision).
 * 
 * This implementation uses a monotone chains and a spatial index to
 * speed up the intersection tests.
 *
 * This implementation appears to be fully robust using an integer
 * precision model.
 *
 * It will function with non-integer precision models, but the
 * results are not 100% guaranteed to be correctly noded.
 *
 * Last port: noding/snapround/MCIndexSnapRounder.java rev. 1.1 (JTS-1.7)
 *
 */
class MCIndexSnapRounder: public Noder { // implments Noder

private:

	// externally owned
	PrecisionModel& pm;

	algorithm::LineIntersector li;

	double scaleFactor;

	SegmentString::NonConstVect* nodedSegStrings;

	auto_ptr<MCIndexPointSnapper> pointSnapper;

	void snapRound(MCIndexNoder& noder, SegmentString::NonConstVect* segStrings);

	
	/**
	 * Computes all interior intersections in the collection of SegmentStrings,
	 * and push their Coordinate to the provided vector.
	 *
	 * Does NOT node the segStrings.
	 *
	 */
	void findInteriorIntersections(MCIndexNoder& noder,
			SegmentString::NonConstVect* segStrings,
			vector<Coordinate>& intersections);

	/**
	 * Computes nodes introduced as a result of snapping segments to snap points (hot pixels)
	 */
	void computeIntersectionSnaps(vector<Coordinate>& snapPts);

	/**
	 * Performs a brute-force comparison of every segment in each {@link SegmentString}.
	 * This has n^2 performance.
	 */
	void computeEdgeVertexSnaps(SegmentString* e);
	
	void checkCorrectness(SegmentString::NonConstVect& inputSegmentStrings);

public:

	MCIndexSnapRounder(PrecisionModel& nPm)
		:
		pm(nPm),
		li(&nPm),
		scaleFactor(nPm.getScale()),
		pointSnapper(0)
	{}

	SegmentString::NonConstVect* getNodedSubstrings() const {
		return SegmentString::getNodedSubstrings(*nodedSegStrings);
	}

	void computeNodes(SegmentString::NonConstVect* segStrings);
 
	/**
	 * Computes nodes introduced as a result of
	 * snapping segments to vertices of other segments
	 *
	 * @param segStrings the list of segment strings to snap together
	 */
	void computeVertexSnaps(SegmentString::NonConstVect& edges);

};




} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/02/23 11:54:20  strk
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
 * Revision 1.7  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.4  2006/02/16 08:41:01  strk
 * added <cmath> include for round()
 *
 * Revision 1.3  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.1  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 *
 **********************************************************************/

