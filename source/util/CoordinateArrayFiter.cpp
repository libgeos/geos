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
 * Revision 1.6  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.5  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.4  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/util.h>
#include <geos/geom.h>

namespace geos {
/**
*  Constructs a <code>CoordinateArrayFilter</code>.
*
*@param  size  the number of points that the <code>CoordinateArrayFilter</code>
*      will collect
*/
CoordinateArrayFilter::CoordinateArrayFilter(int size) {
	n=0;
	pts=new DefaultCoordinateSequence(size);
}

/**
*  Returns the gathered <code>Coordinate</code>s.
*
*@return    the <code>Coordinate</code>s collected by this <code>CoordinateArrayFilter</code>
*/
const CoordinateSequence* CoordinateArrayFilter::getCoordinates() const {
	return pts;
}

void CoordinateArrayFilter::filter_ro(const Coordinate &coord) {
	pts->setAt(coord,n++);
}

void CoordinateArrayFilter::filter_rw(Coordinate &coord) {
	throw new UnsupportedOperationException("CoordinateArrayFilter is a read-only filter");
}

CoordinateArrayFilter::~CoordinateArrayFilter() {
	delete pts;
}

}

