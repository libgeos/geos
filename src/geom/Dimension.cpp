/**********************************************************************
 * $Id: Dimension.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/geom/Dimension.h>
#include <geos/util/IllegalArgumentException.h>

#include <sstream>

using namespace std;

namespace geos {
namespace geom { // geos::geom

/**
 *  Converts the dimension value to a dimension symbol, for example, <code>TRUE => 'T'</code>.
 *
 *@param  dimensionValue  a number that can be stored in the <code>IntersectionMatrix</code>.
 *			Possible values are <code>{TRUE, FALSE, DONTCARE, 0, 1, 2}</code>.
 *@return   a character for use in the string representation of
 *      an <code>IntersectionMatrix</code>. Possible values are <code>{T, F, * , 0, 1, 2}</code>.
 */
char Dimension::toDimensionSymbol(int dimensionValue) {
	switch (dimensionValue) {
		case False:
			return 'F';
		case True:
			return 'T';
		case DONTCARE:
			return '*';
		case P:
			return '0';
		case L:
			return '1';
		case A:
			return '2';
		default:
			ostringstream s;
			s<<"Unknown dimension value: "<<dimensionValue<<endl;
			throw  util::IllegalArgumentException(s.str());
	}
}

/**
 *  Converts the dimension symbol to a dimension value, for example, <code>'*' => DONTCARE</code>.
 *
 *@param  dimensionSymbol  a character for use in the string representation of
 *      an <code>IntersectionMatrix</code>. Possible values are <code>{T, F, * , 0, 1, 2}</code>.
 *@return       a number that can be stored in the <code>IntersectionMatrix</code>.
 *				Possible values are <code>{TRUE, FALSE, DONTCARE, 0, 1, 2}</code>.
 */
int Dimension::toDimensionValue(char dimensionSymbol) {
	switch (dimensionSymbol) {
		case 'F':
		case 'f':
			return False;
		case 'T':
		case 't':
			return True;
		case '*':
			return DONTCARE;
		case '0':
			return P;
		case '1':
			return L;
		case '2':
			return A;
		default:
			ostringstream s;
			s<<"Unknown dimension symbol: "<<dimensionSymbol<<endl;
			throw  util::IllegalArgumentException(s.str());
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

