/**********************************************************************
 * $Id: ScaledNoder.cpp 3319 2011-04-28 15:09:04Z strk $
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

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h> // for apply and delete
#include <geos/geom/CoordinateFilter.h> // for inheritance
#include <geos/noding/ScaledNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/util/math.h>
#include <geos/util.h>

#include <functional>
#include <vector>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#include <string>
#endif

using namespace geos::geom;



namespace geos {
namespace noding { // geos.noding

namespace {

#if GEOS_DEBUG > 1
void
sqlPrint(const std::string& table, std::vector<SegmentString*>& ssv)
{
	std::cerr << "CREATE TABLE \"" << table
		<< "\" (id integer, geom geometry);" << std::endl;

	std::cerr << "COPY \"" << table
		<< "\" FROM stdin;" << std::endl;

	for (size_t i=0, n=ssv.size(); i<n; i++)
	{
		SegmentString* ss=ssv[i];
		geom::CoordinateSequence* cs = ss->getCoordinates();
		assert(cs);

		std::cerr << i << '\t' << "LINESTRING"
			<< *cs
			<< std::endl;
	}
	std::cerr << "\\." << std::endl;
}
#endif // GEOS_DEBUG > 1

} // anonym namespace 

class ScaledNoder::Scaler : public geom::CoordinateFilter {
public:
	const ScaledNoder& sn;
	Scaler(const ScaledNoder&n): sn(n)
	{
#if GEOS_DEBUG
		std::cerr << "Scaler: offsetX,Y: " << sn.offsetX << ","
			<< sn.offsetY << " scaleFactor: " << sn.scaleFactor
			<< std::endl;
#endif
	}

	//void filter_ro(const geom::Coordinate* c) { assert(0); }

	void filter_rw(geom::Coordinate* c) const {
		c->x = util::round( ( c->x - sn.offsetX ) * sn.scaleFactor );
		c->y = util::round( ( c->y - sn.offsetY ) * sn.scaleFactor );
	}

private:
    // Declare type as noncopyable
    Scaler(const Scaler& other);
    Scaler& operator=(const Scaler& rhs);
};

class ScaledNoder::ReScaler: public geom::CoordinateFilter {
public:
	const ScaledNoder& sn;
	ReScaler(const ScaledNoder&n): sn(n)
	{
#if GEOS_DEBUG
		std::cerr << "ReScaler: offsetX,Y: " << sn.offsetX << ","
			<< sn.offsetY << " scaleFactor: " << sn.scaleFactor
			<< std::endl;
#endif
	}

	void filter_ro(const geom::Coordinate* c)
    {
        ::geos::ignore_unused_variable_warning(c);
        assert(0);
    }

	void filter_rw(geom::Coordinate* c) const {
		c->x = c->x / sn.scaleFactor + sn.offsetX;
		c->y = c->y / sn.scaleFactor + sn.offsetY;
	}

private:
    // Declare type as noncopyable
    ReScaler(const ReScaler& other);
    ReScaler& operator=(const ReScaler& rhs);
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

		SegmentString* ss=*i0;

		ss->getCoordinates()->apply_rw(&rescaler);

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
		SegmentString* ss=*i0;

		CoordinateSequence* cs=ss->getCoordinates();

#ifndef NDEBUG
		size_t npts = cs->size();
#endif
		cs->apply_rw(&scaler);
		assert(cs->size() == npts);

		// Actually, we should be creating *new*
		// SegmentStrings here, but who's going
		// to delete them then ? And is it worth
		// the memory cost ?
		cs->removeRepeatedPoints();

	}
}

ScaledNoder::~ScaledNoder()
{
	for (std::vector<geom::CoordinateSequence*>::const_iterator
		it=newCoordSeq.begin(), end=newCoordSeq.end();
		it != end;
		++it)
	{
		delete *it;
	}
}


/*public*/
SegmentString::NonConstVect*
ScaledNoder::getNodedSubstrings() const
{
	SegmentString::NonConstVect* splitSS = noder.getNodedSubstrings();

#if GEOS_DEBUG > 1
	sqlPrint("nodedSegStr", *splitSS);
#endif

	if ( isScaled ) rescale(*splitSS);

#if GEOS_DEBUG > 1
	sqlPrint("scaledNodedSegStr", *splitSS);
#endif

	return splitSS;

}

/*public*/
void
ScaledNoder::computeNodes(SegmentString::NonConstVect* inputSegStr)
{

#if GEOS_DEBUG > 1
	sqlPrint("inputSegStr", *inputSegStr);
#endif

	if (isScaled) scale(*inputSegStr);

#if GEOS_DEBUG > 1
	sqlPrint("scaledInputSegStr", *inputSegStr);
#endif

	noder.computeNodes(inputSegStr);
}





} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.14  2006/06/19 23:33:03  strk
 * Don't *require* CoordinateFilters to define both read-only and read-write methods.
 *
 * Revision 1.13  2006/05/04 14:05:31  strk
 * * source/headers/geos/util/math.h: provided an util::round() method being an inline proxy to call appropriate default rounding function for the whole GEOS codebase. Currently pointing at util::java_math_round() being the last being used.
 *
 * Revision 1.12  2006/05/04 08:30:02  strk
 * removed use of SegmentString::setCoordinates() [dropped]
 *
 * Revision 1.11  2006/05/03 20:05:18  strk
 * Added SQL debugging output
 *
 * Revision 1.10  2006/05/03 15:40:14  strk
 * test SegmentString invariant before and after scaling
 *
 * Revision 1.9  2006/05/03 15:12:35  strk
 * Oops, uninitialized value fix
 *
 * Revision 1.8  2006/05/03 15:00:43  strk
 * Fixed scale() function to remove repeated points *after* rounding. Added brief doxygen class description.
 *
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
