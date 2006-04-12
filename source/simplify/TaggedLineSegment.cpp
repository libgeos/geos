/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/TaggedLineSegment.java rev. 1.1 (JTS-1.7.1)
 *
 **********************************************************************/

#include <geos/simplify/TaggedLineSegment.h>

#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace simplify { // geos::simplify

TaggedLineSegment::TaggedLineSegment(const geom::Coordinate& p0,
			const geom::Coordinate& p1,
			const geom::Geometry* nParent,
			unsigned int nIndex)
	:
	LineSegment(p0, p1),
	parent(nParent),
	index(nIndex)
{
}

TaggedLineSegment::TaggedLineSegment(const geom::Coordinate& p0,
			const geom::Coordinate& p1)
	:
	LineSegment(p0, p1),
	parent(NULL),
	index(0)
{
}

const geom::Geometry* 
TaggedLineSegment::getParent() const
{
	return parent;
}

unsigned int
TaggedLineSegment::getIndex() 
{
	return index;
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/12 14:22:12  strk
 * Initial implementation of TaggedLineSegment and TaggedLineString classes
 *
 **********************************************************************/
