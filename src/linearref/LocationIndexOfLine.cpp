/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: linearref/LengthIndexedLine.java r731
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LocationIndexOfLine.h>
#include <geos/linearref/LocationIndexOfPoint.h>
#include <geos/util/IllegalArgumentException.h>



using namespace geos::geom;

namespace geos {
namespace linearref { // geos.linearref

/* public static */
LinearLocation*
LocationIndexOfLine::indicesOf(const Geometry* linearGeom,
                               const Geometry* subLine)
{
    LocationIndexOfLine locater(linearGeom);
    return locater.indicesOf(subLine);
}

LocationIndexOfLine::LocationIndexOfLine(const Geometry* p_linearGeom) :
    linearGeom(p_linearGeom) {}

/* public */
LinearLocation*
LocationIndexOfLine::indicesOf(const Geometry* subLine) const
{
    const LineString* firstLine = dynamic_cast<const LineString*>(subLine->getGeometryN(0));
    const LineString* lastLine = dynamic_cast<const LineString*>(subLine->getGeometryN(subLine->getNumGeometries() - 1));
    if(! firstLine || !lastLine) {
        throw util::IllegalArgumentException("LocationIndexOfLine::indicesOf only works with geometry collections of LineString");
    }
    const CoordinateXY& startPt = firstLine->getCoordinateN(0);
    const CoordinateXY& endPt = lastLine->getCoordinateN(lastLine->getNumPoints() - 1);

    LocationIndexOfPoint locPt(linearGeom);
    LinearLocation* subLineLoc = new LinearLocation[2];
    subLineLoc[0] = locPt.indexOf(startPt);

    // check for case where subline is zero length
    if(subLine->getLength() == 0.0) {
        subLineLoc[1] = subLineLoc[0];
    }
    else {
        subLineLoc[1] = locPt.indexOfAfter(endPt, &subLineLoc[0]);
    }
    return subLineLoc;
}
}
}
