/**********************************************************************
 * $Id: Location.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>

#include <sstream>

using namespace std;

namespace geos {
namespace geom { // geos::geom

/**
 *  Converts the location value to a location symbol, for example, <code>EXTERIOR => 'e'</code>.
 *
 *@param  locationValue  either EXTERIOR, BOUNDARY, INTERIOR or NULL
 *@return                either 'e', 'b', 'i' or '-'
 */
char Location::toLocationSymbol(int locationValue) {
	switch (locationValue) {
		case EXTERIOR:
			return 'e';
		case BOUNDARY:
			return 'b';
		case INTERIOR:
			return 'i';
		case UNDEF: //NULL
			return '-';
		default:
			ostringstream s;
			s<<"Unknown location value: "<<locationValue;
			throw util::IllegalArgumentException(s.str());
	}
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 **********************************************************************/
