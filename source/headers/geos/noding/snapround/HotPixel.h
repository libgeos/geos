/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/HotPixel.java rev. 1.3 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SNAPROUND_HOTPIXEL_H
#define GEOS_NODING_SNAPROUND_HOTPIXEL_H

#include <geos/export.h>

#include <geos/inline.h>

#include <geos/geom/Coordinate.h> // for composition
#include <geos/geom/Envelope.h> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
	}
	namespace algorithm {
		class LineIntersector;
	}
	namespace noding {
		class NodedSegmentString;
	}
}

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

/** \brief
 * Implements a "hot pixel" as used in the Snap Rounding algorithm.
 *
 * A hot pixel contains the interior of the tolerance square and
 * the boundary
 * <b>minus</b> the top and right segments.
 * 
 * The hot pixel operations are all computed in the integer domain
 * to avoid rounding problems.
 *
 */
class GEOS_DLL HotPixel {

private:

	algorithm::LineIntersector& li;

	geom::Coordinate pt;
	const geom::Coordinate& originalPt;
	geom::Coordinate ptScaled;

	mutable geom::Coordinate p0Scaled;
	mutable geom::Coordinate p1Scaled;

	double scaleFactor;

	double minx;
	double maxx;
	double miny;
	double maxy;

	/** \brief
	 * The corners of the hot pixel
	 * 
	 * In the order:
	 *  1 0
	 *  2 3
	 */
	std::vector<geom::Coordinate> corner;

	/// Owned by this class, constructed on demand
	mutable std::auto_ptr<geom::Envelope> safeEnv; 

	void initCorners(const geom::Coordinate& pt);

	double scale(double val) const;

	void copyScaled(const geom::Coordinate& p,
			geom::Coordinate& pScaled) const;

	/** \brief
	 * Tests whether the segment p0-p1 intersects the hot pixel
	 * tolerance square.
	 *
	 * Because the tolerance square point set is partially open (along the
	 * top and right) the test needs to be more sophisticated than
	 * simply checking for any intersection.  However, it
	 * can take advantage of the fact that because the hot pixel edges
	 * do not lie on the coordinate grid.  It is sufficient to check
	 * if there is at least one of:
	 * 
	 * - a proper intersection with the segment and any hot pixel edge
	 * - an intersection between the segment and both the left
	 *   and bottom edges
	 * - an intersection between a segment endpoint and the hot
	 *   pixel coordinate
	 * 
	 * @param p0
	 * @param p1
	 * @return
	 */
	bool intersectsToleranceSquare(const geom::Coordinate& p0,
			const geom::Coordinate& p1) const;
 

	/** \brief
	 * Test whether the given segment intersects
	 * the closure of this hot pixel.
	 *
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
	bool intersectsPixelClosure(const geom::Coordinate& p0,
			const geom::Coordinate& p1);
 
public:

	HotPixel(const geom::Coordinate& pt,
			double scaleFact,
			algorithm::LineIntersector& li);

	/// \brief
	/// Return reference to original Coordinate
	/// (the one provided at construction time)
	const geom::Coordinate& getCoordinate() const { return originalPt; }

	/** \brief
	 * Returns a "safe" envelope that is guaranteed to contain
	 * the hot pixel. Keeps ownership of it.
	 */
	const geom::Envelope& getSafeEnvelope() const;

	bool intersectsScaled(const geom::Coordinate& p0,
			const geom::Coordinate& p1) const;

	bool intersects(const geom::Coordinate& p0,
			const geom::Coordinate& p1) const;
 
	/**
	 * Adds a new node (equal to the snap pt) to the specified segment
	 * if the segment passes through the hot pixel
	 *
	 * @param segStr
	 * @param segIndex
	 * @return true if a node was added to the segment
	 */
	bool addSnappedNode(NodedSegmentString& segStr, size_t segIndex);

};



} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/noding/snapround/HotPixel.inl"
#endif

#endif // GEOS_NODING_SNAPROUND_HOTPIXEL_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/05/03 17:50:49  strk
 * Doxygen comments
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 **********************************************************************/

