/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2010  Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/LineStringSnapper.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/operation/overlay/snap/LineStringSnapper.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/geom/LineSegment.h>

#include <vector>
#include <memory>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay
namespace snap { // geos.operation.overlay.snap

/*public*/
std::auto_ptr<Coordinate::Vect>
LineStringSnapper::snapTo(const geom::Coordinate::ConstVect& snapPts)
{
	geom::CoordinateList coordList(srcPts);

	snapVertices(coordList, snapPts);
	snapSegments(coordList, snapPts);

	return coordList.toCoordinateArray();
}

/*private*/
void
LineStringSnapper::snapVertices(geom::CoordinateList& srcCoords,
			const geom::Coordinate::ConstVect& snapPts)
{
  if ( srcCoords.empty() ) return;

#if GEOS_DEBUG
cerr << "Snapping vertices of: " << srcCoords << endl;
#endif

	using geom::CoordinateList;

	geom::Coordinate::ConstVect::const_iterator not_found = snapPts.end();

	// try snapping vertices
	// if src is a ring then don't snap final vertex
	CoordinateList::iterator it = srcCoords.begin();
	CoordinateList::iterator end = srcCoords.end(); 
	CoordinateList::iterator last = end; --last;
  if ( isClosed ) --end;
	for ( ; it != end; ++it )
	{
		Coordinate& srcPt = *it;

#if GEOS_DEBUG
cerr << "Checking for a snap for source coordinate " << srcPt << endl;
#endif

		geom::Coordinate::ConstVect::const_iterator found = findSnapForVertex(srcPt, snapPts);
		if ( found == not_found )
		{	// no snaps found (or no need to snap)
#if GEOS_DEBUG
cerr << " no snap found" << endl;
#endif
			continue;
		}

		assert(*found);
		const Coordinate& snapPt = *(*found);
		
#if GEOS_DEBUG
cerr << " found snap point " << snapPt << endl;
#endif

		// update src with snap pt
		*it = snapPt;

#if GEOS_DEBUG
cerr << " source point became " << srcPt << endl;
#endif

		// keep final closing point in synch (rings only)
		if (it == srcCoords.begin() && isClosed)
		{
			*last = snapPt;
		}
	}
}

/*private*/
Coordinate::ConstVect::const_iterator
LineStringSnapper::findSnapForVertex(const Coordinate& pt,
			const Coordinate::ConstVect& snapPts)
{

	// TODO: use std::find_if
	Coordinate::ConstVect::const_iterator end=snapPts.end();
	for ( Coordinate::ConstVect::const_iterator
			it=snapPts.begin();
			it != end;
			++it)
	{
		assert(*it);
		const Coordinate& snapPt = *(*it);

		// shouldn't we look for *all* segments to be snapped rather then a single one?
		if ( snapPt.equals2D(pt) )
		{
#if GEOS_DEBUG
cerr << " points are equal, returning not-found " << endl;
#endif
			return end;
			//continue;
		}

		double dist = snapPt.distance(pt);
#if GEOS_DEBUG
cerr << " distance from snap point " << snapPt << ": " << dist << endl;
#endif

		if ( dist < snapTolerance )
		{
#if GEOS_DEBUG
cerr << " snap point within tolerance, returning iterator to it" << endl;
#endif
			return it;
		}
	}

#if GEOS_DEBUG
cerr << " no snap point within distance, returning not-found" << endl;
#endif

	return end;
}


/*private*/
void
LineStringSnapper::snapSegments(geom::CoordinateList& srcCoords,
			const geom::Coordinate::ConstVect& snapPts)
{

  // nothing to do if there are no source coords..
  if ( srcCoords.empty() ) return;

#if GEOS_DEBUG
cerr << "Snapping segments of: " << srcCoords << endl;
#endif

	for ( Coordinate::ConstVect::const_iterator
			it=snapPts.begin(), end=snapPts.end();
			it != end;
			++it)
	{
		assert(*it);
		const Coordinate& snapPt = *(*it);

#if GEOS_DEBUG
cerr << "Checking for a segment to snap to snapPt " << snapPt << endl;
#endif

		// shouldn't we look for *all* segments to be snapped
		// rather then a single one?
		CoordinateList::iterator too_far = srcCoords.end(); --too_far;
		CoordinateList::iterator segpos =
			findSegmentToSnap(snapPt, srcCoords.begin(), too_far);
		if ( segpos == too_far)
		{
#if GEOS_DEBUG
cerr << " No segment to snap" << endl;
#endif
			continue;
		}
#if GEOS_DEBUG
cerr << " Segment to be snapped found, inserting point" << endl;
#endif
		// insert must happen one-past first point (before next point)
		++segpos;
		srcCoords.insert(segpos, snapPt);
	}

#if GEOS_DEBUG
cerr << " After segment snapping, srcCoors are: " << srcCoords << endl;
#endif

}

/*private*/
/* NOTE: this is called findSegmentIndexToSnap in JTS */
CoordinateList::iterator
LineStringSnapper::findSegmentToSnap(
			const Coordinate& snapPt,
			CoordinateList::iterator from,
			CoordinateList::iterator too_far)
{
	LineSegment seg;
	double minDist = snapTolerance+1; // make sure the first closer then
	                                  // snapTolerance is accepted
	CoordinateList::iterator match=too_far;

	// TODO: use std::find_if
	for ( ; from != too_far; ++from)
	{
		seg.p0 = *from; 
		CoordinateList::iterator to = from;
		++to;
		seg.p1 = *to;

#if GEOS_DEBUG
cerr << " Checking segment " << seg << endl;
#endif

		/**
		 * Check if the snap pt is equal to one of
		 * the segment endpoints.
		 *
		 * If the snap pt is already in the src list,
		 * don't snap at all (unless allowSnappingToSourceVertices
		 * is set to true)
		 */
		if ( seg.p0.equals2D(snapPt) || seg.p1.equals2D(snapPt) )
		{

			if (allowSnappingToSourceVertices) {
#if GEOS_DEBUG
cerr << "   snap point matches a segment endpoint, checking next segment"
     << endl;
#endif
				continue;
			} else {
#if GEOS_DEBUG
cerr << "   snap point matches a segment endpoint, giving up seek" << endl;
#endif
				return too_far;
			}
		}

		double dist = seg.distance(snapPt);
		if ( dist < snapTolerance ) {
      if ( dist < minDist ) {
#if GEOS_DEBUG
cerr << "   snap point distance " << dist << " within tolerance "
     << snapTolerance << " and closer than previous candidate " << minDist
     << endl;
#endif
        match = from;
        minDist = dist;
      }
#if GEOS_DEBUG
      else {
cerr << "   snap point distance " << dist << " within tolerance "
     << snapTolerance << " but not closer than previous candidate " << minDist
     << endl;
      }
#endif
    }
#if GEOS_DEBUG
    else {
cerr << "   snap point distance " << dist << " bigger than tolerance "
     << snapTolerance << endl;
    }
#endif
	}

	return match;
}

} // namespace geos.operation.snap
} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

