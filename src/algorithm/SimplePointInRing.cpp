/**********************************************************************
 * $Id: SimplePointInRing.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/SimplePointInRing.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/LinearRing.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace algorithm { // geos.algorithm

SimplePointInRing::SimplePointInRing(geom::LinearRing *ring){
	pts=ring->getCoordinatesRO();
}

SimplePointInRing::~SimplePointInRing(){
}

bool SimplePointInRing::isInside(const geom::Coordinate& pt){
	return CGAlgorithms::isPointInRing(pt,pts);
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.17  2006/03/09 16:46:46  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.16  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 **********************************************************************/

