/**********************************************************************
 * $Id$
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
 **********************************************************************
 * $Log$
 * Revision 1.14  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.13  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.12  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.11  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.10  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

#include <geos/geom.h>
#include <geos/util.h>
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
}

