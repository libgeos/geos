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
 **********************************************************************/

#include <geos/geom.h>
#include <sstream>
//#include <stdio.h>

namespace geos {

	Coordinate Coordinate::nullCoord=Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber);

/**
 *  Returns a <code>string</code> of the form <I>(x,y,z)</I> .
 *
 *@return    a <code>string</code> of the form <I>(x,y,z)</I>
 */
string Coordinate::toString() const {
	ostringstream s;
	//string result("");
	//char buffer[255];
	if (ISNAN(z)) {
		s<<"("<<x<<","<<y<<")";
		//sprintf(buffer,"(%g,%g)",x,y);
		//result.append(buffer);
		//result.append("");
	} else {
		s<<"("<<x<<","<<y<<","<<z<<")";
		//sprintf(buffer,"(%g,%g,%g)",x,y,z);
		//result.append(buffer);
		//result.append("");
	}
	//return result;
	return s.str();
}

/*
 *  Returns <code>true</code> if <code>a</code> and <code>b</code>
 *  have the same values for x and y.
 *
 *@param  a  a <code>Coordinate</code> with which to do the 3D comparison.
 *@param  b  a <code>Coordinate</code> with which to do the 3D comparison.
 *@return        <code>true</code> if <code>a</code> and <code>b</code>
 *      have the same values for x, y and z.
 */
bool operator==(const Coordinate& a, const Coordinate& b) {
	//return a.equals2D(b);
	if ( a.x != b.x ) return false;
	if ( a.y != b.y ) return false;
	return true;
}

bool operator!=(const Coordinate& a, const Coordinate& b) {
	if ( a.x != b.x ) return true;
	if ( a.y != b.y ) return true;
	return false;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2005/01/28 08:47:06  strk
 * Removed sprintf usage, replaced with sstream
 *
 * Revision 1.19  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.18  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.17  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.16  2004/07/22 07:04:49  strk
 * Documented missing geometry functions.
 *
 * Revision 1.15  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.14  2004/07/14 21:17:10  strk
 * added inequality operator for Coordinate
 *
 * Revision 1.13  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.12  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

