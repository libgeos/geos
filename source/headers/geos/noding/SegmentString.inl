/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTSTRING_INL
#define GEOS_NODING_SEGMENTSTRING_INL

#include <geos/noding/SegmentString.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>

#include <cassert>

namespace geos {
namespace noding { // geos.noding

INLINE
SegmentString::SegmentString(geom::CoordinateSequence *newPts, const void* newContext)
		:
		eiList(this),
		pts(newPts),
		npts(pts->size()),
		context(newContext),
		isIsolatedVar(false)
{
	testInvariant();
}

INLINE
SegmentString::~SegmentString() {}

INLINE const void*
SegmentString::getContext() const
{
	return getData();
}

INLINE const void*
SegmentString::getData() const
{
	testInvariant();
	return context;
}

INLINE const SegmentNodeList&
SegmentString::getNodeList() const
{
	testInvariant();
	return eiList;
}

INLINE SegmentNodeList&
SegmentString::getNodeList()
{
	testInvariant();
	return eiList;
}

INLINE unsigned int
SegmentString::size() const
{
	testInvariant();
	return npts;
}

INLINE const geom::Coordinate&
SegmentString::getCoordinate(unsigned int i) const
{
	testInvariant();
	return pts->getAt(i);
}

INLINE geom::CoordinateSequence*
SegmentString::getCoordinates() const
{
	testInvariant();
	return pts;
}

INLINE void
SegmentString::setIsolated(bool isIsolated)
{
	isIsolatedVar=isIsolated;
	testInvariant();
}

INLINE bool
SegmentString::isIsolated() const
{
	testInvariant();
	return isIsolatedVar;
}

INLINE bool
SegmentString::isClosed() const
{
	testInvariant();
	return pts->getAt(0)==pts->getAt(npts-1);
}

INLINE int
SegmentString::getSegmentOctant(unsigned int index) const
{
	testInvariant();
	if (index == size() - 1) return -1;
	return Octant::octant(getCoordinate(index), getCoordinate(index+1));
}


} // namespace geos.noding
} // namespace geos

#endif // GEOS_NODING_SEGMENTSTRING_INL

