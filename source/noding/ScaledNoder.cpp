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
 * Last port: noding/ScaledNoder.java rev. 1.2 (JTS-1.7)
 *
 **********************************************************************/

#include "geos/noding.h"

namespace geos {

/*public*/
void
ScaledNoder::filter_rw(Coordinate* c) const
{
	c->x = c->x / scaleFactor + offsetX;
	c->y = c->y / scaleFactor + offsetY;
}

/*private*/
void
ScaledNoder::rescale(SegmentString::NonConstVect& segStrings) const
{

	for (SegmentString::NonConstVect::const_iterator
		i0=segStrings.begin(), i0End=segStrings.end();
			i0!=i0End; ++i0)
	{
		//(*i0)->getCoordinates()->applyCoordinateFilter(*this);
		(*i0)->getCoordinates()->apply_rw(this);
	}
}

/*public*/
SegmentString::NonConstVect*
ScaledNoder::getNodedSubstrings() const
{
	SegmentString::NonConstVect* splitSS = noder.getNodedSubstrings();
	if ( isScaled ) rescale(*splitSS);
	return splitSS;

}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 **********************************************************************/
