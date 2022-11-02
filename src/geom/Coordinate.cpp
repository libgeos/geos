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

#include <geos/geom/Coordinate.h>
#include <geos/constants.h> // for std::isnan

#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>


namespace geos {
namespace geom { // geos::geom

CoordinateXY CoordinateXY::_nullCoord = CoordinateXY(DoubleNotANumber, DoubleNotANumber);
Coordinate Coordinate::_nullCoord = Coordinate(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);

CoordinateXY&
CoordinateXY::getNull()
{
    return _nullCoord;
}

Coordinate&
Coordinate::getNull()
{
    return _nullCoord;
}

std::string
Coordinate::toString() const
{
    std::ostringstream s;
    s << std::setprecision(17) << *this;
    return s.str();
}

std::string
CoordinateXY::toString() const
{
    std::ostringstream s;
    s << std::setprecision(17) << *this;
    return s.str();
}

std::ostream&
operator<< (std::ostream& os, const CoordinateXY& c)
{
    os << c.x << " " << c.y;
    return os;
}

std::ostream&
operator<< (std::ostream& os, const Coordinate& c)
{
    if(std::isnan(c.z)) {
        os << c.x << " " << c.y;
    }
    else {
        os << c.x << " " << c.y << " " << c.z;
    }
    return os;
}

} // namespace geos::geom
} // namespace geos

