/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/lineStringSnapper.java rev 1.5 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H
#define GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateList.h>

#include <memory>

// Forward declarations
namespace geos {
	namespace geom { 
		//class PrecisionModel;
		//class CoordinateSequence;
		class CoordinateList;
		class Geometry;
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
	 * Creates a new snapper using the given points
	 * as source points to be snapped.
	 *
	 * @param nSrcPts the points to snap
	 * @param nSnapTolerance the snap tolerance to use
	 */
	LineStringSnapper(const geom::Coordinate::Vect& nSrcPts,
	                                          double nSnapTol)
		:
		srcPts(nSrcPts),
		snapTolerance(nSnapTol)
	{
		size_t s = srcPts.size();
		isClosed = ( s < 2 || srcPts[0].equals2D(srcPts[s-1]) );
	}

	// Snap points are assumed to be all distinct points (a set would be better, uh ?)
	std::auto_ptr<geom::Coordinate::Vect> snapTo(const geom::Coordinate::ConstVect& snapPts);

private:

	const geom::Coordinate::Vect& srcPts;

	double snapTolerance;

	bool isClosed;


	// Modifies first arg
	void snapVertices(geom::CoordinateList& srcCoords,
			const geom::Coordinate::ConstVect& snapPts);


	// Returns snapPts.end() if no snap point is close enough (within snapTol distance)
	geom::Coordinate::ConstVect::const_iterator findSnapForVertex(const geom::Coordinate& pt,
			const geom::Coordinate::ConstVect& snapPts);

	// Modifies first arg
	void snapSegments(geom::CoordinateList& srcCoords,
			const geom::Coordinate::ConstVect& snapPts);

	/// \brief
	/// Finds a src segment which snaps to (is close to) the given snap
	/// point.
	//
	/// Only one segment is determined - this is to prevent
	/// snapping to multiple segments, which would almost certainly cause
	/// invalid geometry to be created.
	/// (The heuristic approach of snapping is really only appropriate
	///  when snap pts snap to a unique spot on the src geometry.)
	///
	///
	/// @param from
	///        an iterator to first point of first segment to be checked
	///
	/// @param too_far
	///        an iterator to last point of last segment to be checked
	///
	/// @returns too_far if no segment needs snapping
	///          (either none within snapTol distance,
	///	      or one fond on the snapPt)
	///
	geom::CoordinateList::iterator findSegmentToSnap(
			const geom::Coordinate& snapPt,
			geom::CoordinateList::iterator from,
			geom::CoordinateList::iterator too_far);

};


} // namespace geos::operation::overlay::snap
} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_OVERLAY_SNAP_LINESTRINGSNAPPER_H

