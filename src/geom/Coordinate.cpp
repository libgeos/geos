/**********************************************************************
 * $Id: Coordinate.cpp 3367 2011-05-17 16:50:03Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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
#include <geos/platform.h> // for ISNAN

#include <sstream>
#include <string>

#ifndef GEOS_INLINE
# include <geos/geom/Coordinate.inl>
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

Coordinate Coordinate::nullCoord=Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber);

Coordinate&
Coordinate::getNull()
{
	return nullCoord;
}

string
Coordinate::toString() const
{
	ostringstream s;
	s<<*this;
	return s.str();
}

std::ostream& operator<< (std::ostream& os, const Coordinate& c)
{
	if ( ISNAN(c.z) )
	{
		os << c.x << " " << c.y;
	} else {
		os << c.x << " " << c.y << " " << c.z;
	}
	return os;
}

} // namespace geos::geom
} // namespace geos

