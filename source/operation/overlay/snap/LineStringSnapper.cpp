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

#include <geos/operation/overlay/snap/LineStringSnapper.h>
#include <geos/geom/CoordinateSequence.h> 

#include <cassert>
#include <limits> // for numeric_limits
#include <memory> // for auto_ptr

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay
namespace snap { // geos.operation.overlay.snap

/*public*/
auto_ptr<CoordinateSequence>
LineStringSnapper::snapTo(const CoordinateSequence& snapPts)
{
	auto_ptr<CoordinateSequence> coordList(snapPts.clone());

	snapVertices(*coordList, snapPts);
	snapSegments(*coordList, snapPts);

	return coordList;
} 

/*public*/
auto_ptr<CoordinateSequence>
LineStringSnapper::snapTo(const vector<const Coordinate*>& snapPts)
{
	auto_ptr<CoordinateSequence> coordList(srcPts.clone());

	snapVertices(*coordList, snapPts);
	snapSegments(*coordList, snapPts);

	return coordList;
} 

/*private*/
void
LineStringSnapper::snapVertices(geom::CoordinateSequence& srcCoords,
                                const geom::CoordinateSequence& snapPts)
{
    // try snapping vertices
    // assume src list has a closing point (is a ring)
    for (size_t i = 0; i < srcCoords.size() - 1; i++) {
      const Coordinate& srcPt = srcCoords.getAt(i);
      const Coordinate* snapVert = findSnapForVertex(srcPt, snapPts);
      if (snapVert != NULL) {
        // update src with snap pt
        srcCoords.setAt(*snapVert, i);
        // keep final closing point in synch (rings only)
        if (i == 0 && isClosed)
          srcCoords.setAt(*snapVert, srcCoords.size() - 1);
      }
    }
}

/*private*/
void
LineStringSnapper::snapVertices(geom::CoordinateSequence& srcCoords,
                                const vector<const Coordinate*>& snapPts)
{
    // try snapping vertices
    // assume src list has a closing point (is a ring)
    for (size_t i = 0; i < srcCoords.size() - 1; i++) {
      const Coordinate& srcPt = srcCoords.getAt(i);
      const Coordinate* snapVert = findSnapForVertex(srcPt, snapPts);
      if (snapVert != NULL) {
        // update src with snap pt
        srcCoords.setAt(*snapVert, i);
        // keep final closing point in synch (rings only)
        if (i == 0 && isClosed)
          srcCoords.setAt(*snapVert, srcCoords.size() - 1);
      }
    }
}

/*private*/
const geom::Coordinate*
LineStringSnapper::findSnapForVertex(const geom::Coordinate& pt,
	                  const geom::CoordinateSequence& snapPts)
{
    for (size_t i = 0; i < snapPts.size(); i++) {
      // if point is already equal to a src pt, don't snap
      if (pt.equals2D(snapPts[i]))
        return 0;
      if (pt.distance(snapPts[i]) < snapTolerance)
        return &(snapPts[i]);
    }
    return 0;
}

/*private*/
const geom::Coordinate*
LineStringSnapper::findSnapForVertex(const geom::Coordinate& pt,
			 const vector<const Coordinate*>& snapPts)
{
    for (size_t i = 0; i < snapPts.size(); i++) {
      // if point is already equal to a src pt, don't snap
      if (pt.equals2D(*snapPts[i]))
        return 0;
      if (pt.distance(*snapPts[i]) < snapTolerance)
        return snapPts[i];
    }
    return 0;
}

/*private*/
void
LineStringSnapper::snapSegments(geom::CoordinateSequence& srcCoords,
	                  const geom::CoordinateSequence& snapPts)
{
    int distinctPtCount = snapPts.size();

    // check for duplicate snap pts.
    // Need to do this better - need to check all points for dups (using a Set?)
    if (snapPts[0].equals2D(snapPts[snapPts.size() - 1]))
        distinctPtCount = snapPts.size() - 1;

    for (int i = 0; i < distinctPtCount; i++) {
      const Coordinate& snapPt = snapPts[i];
      int index = findSegmentIndexToSnap(snapPt, srcCoords);
      /**
       * If a segment to snap to was found, "crack" it at the snap pt.
       * The new pt is inserted immediately into the src segment list,
       * so that subsequent snapping will take place on the latest segments.
       * Duplicate points are not added.
       */
      if (index >= 0) {
        srcCoords.add(index + 1, snapPt, false);
      }
    }
}

/*private*/
void
LineStringSnapper::snapSegments(geom::CoordinateSequence& srcCoords,
                          const vector<const Coordinate*>& snapPts)
{
    int distinctPtCount = snapPts.size();

    // check for duplicate snap pts.
    // Need to do this better - need to check all points for dups (using a Set?)
    if (snapPts[0]->equals2D(*snapPts[snapPts.size() - 1]))
        distinctPtCount = snapPts.size() - 1;

    for (int i = 0; i < distinctPtCount; i++) {
      const Coordinate& snapPt = *(snapPts[i]);
      int index = findSegmentIndexToSnap(snapPt, srcCoords);
      /**
       * If a segment to snap to was found, "crack" it at the snap pt.
       * The new pt is inserted immediately into the src segment list,
       * so that subsequent snapping will take place on the latest segments.
       * Duplicate points are not added.
       */
      if (index >= 0) {
        srcCoords.add(index + 1, snapPt, false);
      }
    }
}

/*private*/
int
LineStringSnapper::findSegmentIndexToSnap(const geom::Coordinate& snapPt,
	                      const geom::CoordinateSequence& srcCoords)
{
    double minDist = numeric_limits<double>::max();
    int snapIndex = -1;
    for (size_t i = 0; i < srcCoords.size() - 1; i++) {
      seg.p0 = srcCoords.getAt(i);
      seg.p1 = srcCoords.getAt(i + 1);

      /**
       * If the snap pt is already in the src list, don't snap
       */
      if (seg.p0.equals2D(snapPt) || seg.p1.equals2D(snapPt))
        return -1;

      double dist = seg.distance(snapPt);
      if (dist < snapTolerance && dist < minDist) {
        minDist = dist;
        snapIndex = i;
      }
    }
    return snapIndex;
}

} // namespace geos.operation.snap
} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

