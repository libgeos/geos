/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_UNIQUECOORDINATEARRAYFILTER_H
#define GEOS_UTIL_UNIQUECOORDINATEARRAYFILTER_H

#include <cassert>
#include <set>
#include <vector>

#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>

namespace geos {
namespace util { // geos::util

/*
 *  A CoordinateFilter that fills a vector of Coordinate const pointers.
 *  The set of coordinates contains no duplicate points.
 *
 *  Last port: util/UniqueCoordinateArrayFilter.java rev. 1.17
 */
class UniqueCoordinateArrayFilter: public geom::CoordinateFilter {

private:
	geom::Coordinate::ConstVect &pts;	// target set reference
	geom::Coordinate::ConstSet uniqPts; 	// unique points set

public:
	/**
	 * Constructs a CoordinateArrayFilter.
	 *
	 * @param  target   The destination set. 
	 */
	UniqueCoordinateArrayFilter(geom::Coordinate::ConstVect &target)
		:
		pts(target)
		{}

	virtual ~UniqueCoordinateArrayFilter() {}

	virtual void filter_ro(const geom::Coordinate *coord)
	{
		if ( uniqPts.insert(coord).second )
			pts.push_back(coord);
	}

	virtual void filter_rw(geom::Coordinate *coord) const
	{
		// UniqueCoordinateArrayFilter is a read-only filter
		assert(0); 
	}
};

} // namespace geos::util
} // namespace geos

#endif // GEOS_UTIL_UNIQUECOORDINATEARRAYFILTER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
