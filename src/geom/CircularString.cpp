/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/CircularString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace geom {

CircularString::~CircularString() = default;

std::string
CircularString::getGeometryType() const
{
    return "CircularString";
}

GeometryTypeId
CircularString::getGeometryTypeId() const
{
    return GEOS_CIRCULARSTRING;
}

std::unique_ptr<CircularString>
CircularString::clone() const
{
    return std::unique_ptr<CircularString>(cloneImpl());
}

CircularString*
CircularString::reverseImpl() const
{
    if (isEmpty()) {
        return clone().release();
    }

    assert(points.get());
    auto seq = points->clone();
    seq->reverse();
    assert(getFactory());
    return getFactory()->createCircularString(std::move(seq)).release();
}

}
}
