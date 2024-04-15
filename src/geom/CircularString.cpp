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
#include <geos/util/UnsupportedOperationException.h>

namespace geos {
namespace geom {

/*public*/
CircularString::CircularString(std::unique_ptr<CoordinateSequence>&& newCoords,
                               const GeometryFactory& factory)
    :
    SimpleCurve(std::move(newCoords), false, factory)
{
    validateConstruction();
}

CircularString::~CircularString() = default;

std::unique_ptr<CircularString>
CircularString::clone() const
{
    return std::unique_ptr<CircularString>(cloneImpl());
}

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

void
CircularString::validateConstruction()
{
    if (points.get() == nullptr) {
        points = std::make_unique<CoordinateSequence>();
        return;
    }

    if (points->size() == 2) {
        throw util::IllegalArgumentException("point array must contain 0 or >2 elements\n");
    }
}

}
}
