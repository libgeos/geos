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


#include <sstream>
#include <geos/geom.h>
#include <geos/util.h>

namespace geos {

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
			throw  IllegalArgumentException(s.str());
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
			throw  IllegalArgumentException(s.str());
	}
}
}

