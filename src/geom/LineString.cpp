/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/LineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/util/IllegalArgumentException.h>
#include <geos/algorithm/Length.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h> // for getBoundary
#include <geos/geom/Envelope.h>
#include <geos/operation/BoundaryOp.h>
#include <geos/util.h>

#include <algorithm>
#include <typeinfo>
#include <memory>
#include <cassert>

using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

LineString::~LineString(){}

/*protected*/
LineString::LineString(const LineString& ls)
    : SimpleCurve(ls)
{
}

/*public*/
LineString::LineString(CoordinateSequence::Ptr && newCoords,
                       const GeometryFactory& factory)
    :
    SimpleCurve(std::move(newCoords), true, factory)
{
    validateConstruction();
}

LineString*
LineString::reverseImpl() const
{
    if(isEmpty()) {
        return clone().release();
    }

    assert(points.get());
    auto seq = points->clone();
    seq->reverse();
    assert(getFactory());
    return getFactory()->createLineString(std::move(seq)).release();
}


/*private*/
void
LineString::validateConstruction()
{
    if(points.get() == nullptr) {
        points = std::make_unique<CoordinateSequence>();
        return;
    }

    if(points->size() == 1) {
        throw util::IllegalArgumentException("point array must contain 0 or >1 elements\n");
    }
}


std::string
LineString::getGeometryType() const
{
    return "LineString";
}


double
LineString::getLength() const
{
    return Length::ofLine(points.get());
}


GeometryTypeId
LineString::getGeometryTypeId() const
{
    return GEOS_LINESTRING;
}

} // namespace geos::geom
} // namespace geos
