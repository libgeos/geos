/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SegmentNode.java 4667170ea (JTS-1.17)
 *
 **********************************************************************/

#include <iostream>
#include <sstream>
#include <iomanip>

#include <geos/noding/SegmentNode.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/geom/Coordinate.h>


using geos::geom::CoordinateXYZM;
using geos::geom::Coordinate;

namespace geos {
namespace noding { // geos.noding

std::ostream&
operator<< (std::ostream& os, const SegmentNode& n)
{
    return os << n.coord << " seg#=" << n.segmentIndex << " octant#=" << n.segmentOctant << std::endl;
}


} // namespace geos.noding
} // namespace geos

