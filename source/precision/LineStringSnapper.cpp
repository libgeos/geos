/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: precision/LineStringSnapper.java rev. 0
 *
 **********************************************************************/

#include <geos/precision/LineStringSnapper.h>
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

#ifdef GEOS_DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

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
	using geom::CoordinateList;

	geom::Coordinate::ConstVect::const_iterator not_found = snapPts.end();

	// try snapping vertices
	// assume src list has a closing point (is a ring)
	CoordinateList::iterator it = srcCoords.begin();
	CoordinateList::iterator end = srcCoords.end(); 
	CoordinateList::iterator last = end; --last;
	for ( ; it != end; ++it )
	{
		Coordinate& srcPt = *it;

#ifdef GEOS_DEBUG
cerr << "Checking for a snap for source coordinate " << srcPt << endl;
#endif

		geom::Coordinate::ConstVect::const_iterator found = findSnapForVertex(srcPt, snapPts);
		if ( found == not_found )
		{	// no snaps found (or no need to snap)
#ifdef GEOS_DEBUG
cerr << "No snap found" << endl;
#endif
			continue;
		}

		assert(*found);
		const Coordinate& snapPt = *(*found);
		
#ifdef GEOS_DEBUG
cerr << "Found snap point " << snapPt << endl;
#endif

		// update src with snap pt
		*it = snapPt;

#ifdef GEOS_DEBUG
cerr << "Source point became " << srcPt << endl;
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

#ifdef GEOS_DEBUG
cerr << " misuring distance between snap point " << snapPt << " and source point " << pt << endl;
#endif

		// shouldn't we look for *all* segments to be snapped rather then a single one?
		if ( snapPt.equals2D(pt) )
		{
#ifdef GEOS_DEBUG
cerr << " points are equal, returning not-found " << endl;
#endif
			return end;
			//continue;
		}

		double dist = snapPt.distance(pt);
		if ( dist <= snapTolerance )
		{
#ifdef GEOS_DEBUG
cerr << " points are within distance (" << dist << ") returning iterator to snap point" << endl;
#endif
			return it;
		}
	}

#ifdef GEOS_DEBUG
cerr << " No snap point within distance, returning not-found" << endl;
#endif

	return end;
}


/*private*/
void
LineStringSnapper::snapSegments(geom::CoordinateList& srcCoords,
			const geom::Coordinate::ConstVect& snapPts)
{

#ifdef GEOS_DEBUG
cerr << " Snapping segment from: " << srcCoords << endl;
#endif

	for ( Coordinate::ConstVect::const_iterator
			it=snapPts.begin(), end=snapPts.end();
			it != end;
			++it)
	{
		assert(*it);
		const Coordinate& snapPt = *(*it);

#ifdef GEOS_DEBUG
cerr << "Checking for a segment to snap to snapPt " << snapPt << endl;
#endif

		// shouldn't we look for *all* segments to be snapped
		// rather then a single one?
		CoordinateList::iterator too_far = srcCoords.end(); --too_far;
		CoordinateList::iterator segpos =
			findSegmentToSnap(snapPt, srcCoords.begin(), too_far);
		if ( segpos == too_far)
		{
#ifdef GEOS_DEBUG
cerr << " No segment to snap" << endl;
#endif
			continue;
		}
#ifdef GEOS_DEBUG
cerr << " Segment to be snapped found, inserting point" << endl;
#endif
		// insert must happen one-past first point (before next point)
		++segpos;
		srcCoords.insert(segpos, snapPt);
	}

#ifdef GEOS_DEBUG
cerr << " After segment snapping, srcCoors are: " << srcCoords << endl;
#endif

}

/*private*/
CoordinateList::iterator
LineStringSnapper::findSegmentToSnap(
			const Coordinate& snapPt,
			CoordinateList::iterator from,
			CoordinateList::iterator too_far)
{
	LineSegment seg;

	// TODO: use std::find_if
	for ( ; from != too_far; ++from)
	{
		seg.p0 = *from; 
		CoordinateList::iterator to = from;
		++to;
		seg.p1 = *to;

#ifdef GEOS_DEBUG
cerr << " Checking segment " << seg << " for snapping against point" << snapPt << endl;
#endif

		if ( seg.p0.equals2D(snapPt) || seg.p1.equals2D(snapPt) )
		{

#ifdef GEOS_DEBUG
cerr << " One of segment endpoints equal snap point, checkin next" << endl;
#endif
			// no snapping needed, check next
			continue;
		}

		double dist = seg.distance(snapPt);
		if ( dist <= snapTolerance )
		{
#ifdef GEOS_DEBUG
cerr << " Segment/snapPt distance within tolerance (" << dist << ") returning iterator to start point" << endl;
#endif
			return from;
		}
	}

#ifdef GEOS_DEBUG
cerr << " No segment within distance" << endl;
#endif

	return too_far; // not found
}

} // namespace geos.precision
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/07/21 17:09:14  strk
 * Added new precision::LineStringSnapper class + test
 * and precision::GeometrySnapper (w/out test)
 *
 **********************************************************************/

