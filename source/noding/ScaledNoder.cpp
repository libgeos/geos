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
 * Last port: noding/ScaledNoder.java rev. 1.3 (JTS-1.7.1)
 *
 **********************************************************************/

#include <functional>
#include <vector>
#include <cassert>

#include "geos/geom/Coordinate.h"
#include "geos/geom/CoordinateSequence.h" // for apply
#include "geos/noding/ScaledNoder.h"
#include "geos/noding/SegmentString.h"
#include "geos/util/math.h"

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

class ScaledNoder::Scaler: public geom::CoordinateFilter {
public:
	const ScaledNoder& sn;
	Scaler(const ScaledNoder&n): sn(n) {}
	void filter_ro(const geom::Coordinate* c) { assert(0); }
	void filter_rw(geom::Coordinate* c) const {
		c->x = util::java_math_round( ( c->x - sn.offsetX ) *  sn.scaleFactor );
		c->y = util::java_math_round( ( c->y - sn.offsetY ) *  sn.scaleFactor );
	}
};

class ScaledNoder::ReScaler: public geom::CoordinateFilter {
public:
	const ScaledNoder& sn;
	ReScaler(const ScaledNoder&n): sn(n) {}
	void filter_ro(const geom::Coordinate* c) { assert(0); }
	void filter_rw(geom::Coordinate* c) const {
		c->x = c->x / sn.scaleFactor + sn.offsetX;
		c->y = c->y / sn.scaleFactor + sn.offsetY;
	}
};

/*private*/
void
ScaledNoder::rescale(SegmentString::NonConstVect& segStrings) const
{
	ReScaler rescaler(*this);
	for (SegmentString::NonConstVect::const_iterator
		i0=segStrings.begin(), i0End=segStrings.end();
			i0!=i0End; ++i0)
	{
		//(*i0)->getCoordinates()->applyCoordinateFilter(*this);
		(*i0)->getCoordinates()->apply_rw(&rescaler);
	}
}


/*private*/
void
ScaledNoder::scale(SegmentString::NonConstVect& segStrings) const
{
	Scaler scaler(*this);
	for (SegmentString::NonConstVect::const_iterator
		i0=segStrings.begin(), i0End=segStrings.end();
			i0!=i0End; ++i0)
	{
		//(*i0)->getCoordinates()->applyCoordinateFilter(*this);
		(*i0)->getCoordinates()->removeRepeatedPoints().apply_rw(&scaler);
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

/*public*/
void
ScaledNoder::computeNodes(SegmentString::NonConstVect* inputSegStr)
{
	if (isScaled) scale(*inputSegStr);
	noder.computeNodes(inputSegStr);
}





} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/05/03 09:14:22  strk
 * * source/operation/buffer/OffsetCurveSetBuilder.cpp: used auto_ptr to protect leaks of CoordinateSequence
 * * source/noding/ScaledNoder.cpp, source/headers/geos/noding/ScaledNoder.h: ported JTS bugfix in scale method.
 *
 * Revision 1.6  2006/05/02 16:22:18  strk
 * * source/noding/ScaledNoder.cpp: use java_math_round instead of sym_round.
 *
 * Revision 1.5  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.4  2006/03/13 21:19:51  strk
 * Fixed bug in ScaledNoder scaling mechanism (hugly code, due to CoordinateSequence visitor pattern design). Tests are still failing so this possibly needs some other fix. Streamlined includes by implementation file.
 *
 * Revision 1.3  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.2  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
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
