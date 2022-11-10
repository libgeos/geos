/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009 Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/BasicSegmentString.java rev. 1.1 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/BasicSegmentString.h>
#include <geos/geom/CoordinateSequence.h>

#include <iostream>
#include <sstream>

namespace geos {
namespace noding { // geos.noding

/* public virtual */
std::ostream&
BasicSegmentString::print(std::ostream& os) const
{
    os << "BasicSegmentString: " << std::endl;
    os << " LINESTRING" << *(getCoordinates()) << ";" << std::endl;

    return os;
}


} // namespace geos.noding
} // namespace geos

