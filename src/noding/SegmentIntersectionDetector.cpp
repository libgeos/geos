/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/noding/SegmentIntersectionDetector.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/SegmentString.h>

using geos::geom::CoordinateXY;

namespace geos {
namespace noding { // geos::noding



void
SegmentIntersectionDetector::
processIntersections(
    noding::SegmentString* e0, std::size_t segIndex0,
    noding::SegmentString* e1, std::size_t segIndex1)
{
    // don't bother intersecting a segment with itself
    if(e0 == e1 && segIndex0 == segIndex1) {
        return;
    }

    const CoordinateXY& p00 = e0->getCoordinate<CoordinateXY>( segIndex0 );
    const CoordinateXY& p01 = e0->getCoordinate<CoordinateXY>( segIndex0 + 1 );
    const CoordinateXY& p10 = e1->getCoordinate<CoordinateXY>( segIndex1 );
    const CoordinateXY& p11 = e1->getCoordinate<CoordinateXY>( segIndex1 + 1 );

    li->computeIntersection(p00, p01, p10, p11);

    if(li->hasIntersection()) {
        // record intersection info
        _hasIntersection = true;

        bool isProper = li->isProper();

        if(isProper) {
            _hasProperIntersection = true;
        }
        else {
            _hasNonProperIntersection = true;
        }

        // If this is the kind of intersection we are searching for
        // OR no location has yet been recorded
        // save the location data
        bool saveLocation = true;

        if(findProper && !isProper) {
            saveLocation = false;
        }

        if(!intPt || saveLocation) {
            // record intersection location (approximate)
            intPt = &li->getIntersection(0);

            delete intSegments;

            // record intersecting segments
            intSegments = new geom::CoordinateSequence();
            intSegments->add(p00, true);
            intSegments->add(p01, true);
            intSegments->add(p10, true);
            intSegments->add(p11, true);
        }
    }
}


} // geos::noding
} // geos
