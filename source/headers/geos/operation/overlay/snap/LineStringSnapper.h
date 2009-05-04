/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/LineStringSnapper.java rev 1.5 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H
#define GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H

#include <geos/geom/LineSegment.h> // for composition
#include <geos/geom/LineString.h> // for inlined ctor

#include <vector>
#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateSequence;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay
namespace snap { // geos::operation::overlay::snap

/** \brief
 * Snaps the vertices and segments of a LineString to a set
 * of target snap vertices.
 *
 * A snapping distance tolerance is used to control where snapping is performed.
 *
 */
class LineStringSnapper {

public:

	/**
	 * Creates a new snapper using the points in the given LineString
	 * as source snap points.
	 *
	 * @param srcLline a LineString to snap
	 * @param nSnapTolerance the snap tolerance to use
	 */
	LineStringSnapper(const geom::LineString& srcLline,
	                  double nSnapTolerance)
		:
		snapTolerance(nSnapTolerance),
		srcPts(*(srcLline.getCoordinatesRO())),
		seg(),
		isClosed(false)
	{
	}

	/**
	 * Creates a new snapper using the given points
	 * as source points to be snapped.
	 *
	 * @param nSrcPts the points to snap
	 * @param nSnapTolerance the snap tolerance to use
	 */
	LineStringSnapper(const geom::CoordinateSequence& nSrcPts,
	                  double nSnapTolerance)
		:
		snapTolerance(nSnapTolerance),
		srcPts(nSrcPts),
		seg(),
		isClosed(false)
	{
	}

	/**
	 * Snaps the vertices and segments of the source points
	 * to the given set of target snap points.
	 *
	 * @param snapPts the vertices to snap to
	 * @return a sequence formed by the snapped points
	 */
	std::auto_ptr<geom::CoordinateSequence> snapTo(
	                 const geom::CoordinateSequence& snapPts);

	std::auto_ptr<geom::CoordinateSequence> snapTo(
	                 const std::vector<const geom::Coordinate*>& snapPts);

private:

	/**
	 * Snap source vertices to vertices in the target.
	 *
	 * @param srcCoords the points to snap
	 * @param snapPts the points to snap to
	 */
	void snapVertices(geom::CoordinateSequence& srcCoords,
	                  const geom::CoordinateSequence& snapPts);

	void snapVertices(geom::CoordinateSequence& srcCoords,
	                 const std::vector<const geom::Coordinate*>& snapPts);


	// return pointer into snapPts memory, or null
	const geom::Coordinate*
	findSnapForVertex(const geom::Coordinate& pt,
	                  const geom::CoordinateSequence& snapPts);

	const geom::Coordinate*
	findSnapForVertex(const geom::Coordinate& pt,
	                 const std::vector<const geom::Coordinate*>& snapPts);

	/**
	 * Snap segments of the source to nearby snap vertices.
	 * Source segments are "cracked" at a snap vertex, and further
	 * snapping takes place on the modified list of segments.
	 * For each distinct snap vertex, at most one source segment
	 * is snapped to.  This prevents "cracking" multiple segments
	 * at the same point, which would almost certainly cause the result
	 * to be invalid.
	 *
	 * @param srcCoords
	 * @param snapPts
	 */
	void snapSegments(geom::CoordinateSequence& srcCoords,
	                  const geom::CoordinateSequence& snapPts);

	void snapSegments(geom::CoordinateSequence& srcCoords,
	                 const std::vector<const geom::Coordinate*>& snapPts);

	/** \brief
	 * Finds a src segment which snaps to (is close to)
	 * the given snap point.
	 *
	 * Only one segment is determined - this is to prevent                           * snapping to multiple segments, which would almost certainly
	 * cause invalid geometry to be created.
	 * (The heuristic approach of snapping is really only appropriate when
	 * snap pts snap to a unique spot on the src geometry.)
	 *
	 * @param snapPt the point to snap to
	 * @param srcCoords the source segment coordinates
	 * @return the index of the snapped segment
	 * @return -1 if no segment snaps
	 */
	int findSegmentIndexToSnap(const geom::Coordinate& snapPt,
	                      const geom::CoordinateSequence& srcCoords);
 

	double snapTolerance;

	const geom::CoordinateSequence& srcPts;

	geom::LineSegment seg; // for reuse during snapping

	bool isClosed;

};

} // namespace geos::operation::overlay::snap
} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H

/**********************************************************************
 * $Log$
 **********************************************************************/

