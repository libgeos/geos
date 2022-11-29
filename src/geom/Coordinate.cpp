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

const CoordinateXY CoordinateXY::_nullCoord = CoordinateXY(DoubleNotANumber, DoubleNotANumber);
const Coordinate Coordinate::_nullCoord = Coordinate(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);
const CoordinateXYM CoordinateXYM::_nullCoord = CoordinateXYM(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);
const CoordinateXYZM CoordinateXYZM::_nullCoord = CoordinateXYZM(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);

const CoordinateXY&
CoordinateXY::getNull()
{
    return _nullCoord;
}

const Coordinate&
Coordinate::getNull()
{
    return _nullCoord;
}

const CoordinateXYM&
CoordinateXYM::getNull()
{
    return _nullCoord;
}

const CoordinateXYZM&
CoordinateXYZM::getNull()
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

std::string
CoordinateXYM::toString() const
{
    std::ostringstream s;
    s << std::setprecision(17) << *this;
    return s.str();
}

std::string
CoordinateXYZM::toString() const
{
    std::ostringstream s;
    s << std::setprecision(17) << *this;
    return s.str();
}

std::ostream&
operator<< (std::ostream& os, const CoordinateType typ)
{
    switch(typ) {
        case CoordinateType::XY: os << "XY"; break;
        case CoordinateType::XYZ: os << "XYZ"; break;
        case CoordinateType::XYM: os << "XYM"; break;
        case CoordinateType::XYZM: os << "XYZM"; break;
    }

    return os;
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
    os << c.x << " " << c.y;
    if(!std::isnan(c.z)) {
        os << " " << c.z;
    }
    return os;
}

std::ostream&
operator<< (std::ostream& os, const CoordinateXYM& c)
{
    os << c.x << " " << c.y;
    if(!std::isnan(c.m)) {
        os << " " << c.m;
    }
    return os;
}

std::ostream&
operator<< (std::ostream& os, const CoordinateXYZM& c)
{
    os << c.x << " " << c.y;
    if(!std::isnan(c.z) || !std::isnan(c.m)) {
        os << " " << c.z;
    }
    if(!std::isnan(c.m)) {
        os << " " << c.m;
    }
    return os;
}

} // namespace geos::geom
} // namespace geos

