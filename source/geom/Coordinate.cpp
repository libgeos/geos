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


#include <geos/geom.h>
#include <stdio.h>

namespace geos {

	Coordinate Coordinate::nullCoord=Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber);

//void Coordinate::setNull() {
//	x=DoubleNotANumber;
//	y=DoubleNotANumber;
//	z=DoubleNotANumber;
//}
//
//Coordinate& Coordinate::getNull() {
//	return *(new Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber));
//}
//
///**
// *  Default <code>Coordinate</code> constructor.
// *  Constructs a <code>Coordinate</code> at (0,0,NaN).
// */
//Coordinate::Coordinate() {
//	x=0.0;
//	y=0.0;
//	z=DoubleNotANumber;
//}
//
///**
// *  Constructs a <code>Coordinate</code> at (xNew,yNew,zNew).
// *
// *@param  xNew  the x-value
// *@param  yNew  the y-value
// *@param  zNew  the z-value
// */
//Coordinate::Coordinate(double xNew, double yNew, double zNew) {
//	x=xNew;
//	y=yNew;
//	z=zNew;
//}
//
///**
// *  Constructs a <code>Coordinate</code> having the same (x,y,z) values as
// *  <code>other</code>. Replaces clone() method.
// *
// *@param  c  the <code>Coordinate</code> to copy.
// */
//Coordinate::Coordinate(const Coordinate& c){
//	x=c.x;
//	y=c.y;
//	z=c.z;
//}
//
///**
// *  Constructs a <code>Coordinate</code> at (xNew,yNew,NaN).
// *
// *@param  xNew  the x-value
// *@param  yNew  the y-value
// */
//Coordinate::Coordinate(double xNew, double yNew){
//	x=xNew;
//	y=yNew;
//	z=DoubleNotANumber;
//}
//
///**
// *  Default <code>Coordinate</code> destructor.
// */
////Coordinate::~Coordinate(){}

///**
// *  Sets this <code>Coordinate</code>s (x,y,z) values to that of <code>other</code>.
// *
// *@param  other  the <code>Coordinate</code> to copy
// */
//void Coordinate::setCoordinate(Coordinate& other) {
//    x = other.x;
//    y = other.y;
//    z = other.z;
//}
//
///**
// *  Returns whether the planar projections of the two <code>Coordinate</code>s
// *  are equal.
// *
// *@param  other  a <code>Coordinate</code> with which to do the 2D comparison.
// *@return        <code>true</code> if the x- and y-coordinates are equal; the
// *      z-coordinates do not have to be equal.
// */
//bool Coordinate::equals2D(Coordinate& other) {
//    if (x != other.x) {
//      return false;
//    }
//    if (y != other.y) {
//      return false;
//    }
//    return true;
//}
//
///**
// *  Compares this object with the specified object for order.
// *  Since Coordinates are 2.5D, this routine ignores the z value when making the comparison.
// *  Returns
// *  <UL>
// *    <LI> -1 : this.x < other.x || ((this.x == other.x) && (this.y <
// *    other.y))
// *    <LI> 0 : this.x == other.x && this.y = other.y
// *    <LI> 1 : this.x > other.x || ((this.x == other.x) && (this.y > other.y))
// *
// *  </UL>
// *
// *
// *@param  o  the <code>Coordinate</code> with which this <code>Coordinate</code>
// *      is being compared
// *@return    a negative integer, zero, or a positive integer as this <code>Coordinate</code>
// *      is less than, equal to, or greater than the specified <code>Coordinate</code>
// */
//int Coordinate::compareTo(Coordinate& other) {
//    if (x < other.x) {
//      return -1;
//    }
//    if (x > other.x) {
//      return 1;
//    }
//    if (y < other.y) {
//      return -1;
//    }
//    if (y > other.y) {
//      return 1;
//    }
//    return 0;
//}
//
///**
// *  Returns <code>true</code> if <code>other</code> has the same values for x,
// *  y and z.
// *
// *@param  other  a <code>Coordinate</code> with which to do the 3D comparison.
// *@return        <code>true</code> if <code>other</code> is a <code>Coordinate</code>
// *      with the same values for x, y and z.
// */
//bool Coordinate::equals3D(Coordinate& other) {
//    return (x == other.x) && ( y == other.y) && (( z == other.z)||(z==DoubleNotANumber && other.z==DoubleNotANumber));
//}
//
/**
 *  Returns a <code>string</code> of the form <I>(x,y,z)</I> .
 *
 *@return    a <code>string</code> of the form <I>(x,y,z)</I>
 */
string Coordinate::toString() const {
	string result("");
	char buffer[255];
	if (z==DoubleNotANumber) {
		sprintf(buffer,"(%g,%g)",x,y);
		result.append(buffer);
		result.append("");
	} else {
		sprintf(buffer,"(%g,%g,%g)",x,y,z);
		result.append(buffer);
		result.append("");
	}
	return result;
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
	//return (a.x == b.x) && ( a.y == b.y) && (( a.z == b.z)||(a.z==DoubleNotANumber && b.z==DoubleNotANumber));
	if ( a.x != b.x ) return false;
	if ( a.y != b.y ) return false;
	if ( a.z != b.z ) return false;
	return true;
}

bool operator!=(const Coordinate& a, const Coordinate& b) {
	if ( a.x != b.x ) return true;
	if ( a.y != b.y ) return true;
	if ( a.z != b.z ) return true;
	return false;
}
}
