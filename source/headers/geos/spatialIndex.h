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
 * Revision 1.5  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.4  2004/10/26 17:46:18  strk
 * Removed slash-stars in comments to remove annoying compiler warnings.
 *
 * Revision 1.3  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.6  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.5  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEX_H
#define GEOS_INDEX_H

#include <memory>
#include <geos/platform.h>
#include <geos/geom.h>

using namespace std;

namespace geos {

/*
 * The basic insertion and query operations supported by classes
 * implementing spatial index algorithms.
 * <p>
 * A spatial index typically provides a primary filter for range rectangle queries. A
 * secondary filter is required to test for exact intersection. Of course, this
 * secondary filter may consist of other tests besides intersection, such as
 * testing other kinds of spatial relationships.
 *
 */
class SpatialIndex {
public:
	virtual ~SpatialIndex() {};

	/*
	 * Adds a spatial item with an extent specified by the given Envelope
	 * to the index
	 */
	virtual void insert(const Envelope *itemEnv, void *item)=0;

	/*
	 * Queries the index for all items whose extents intersect the given search Envelope
	 * Note that some kinds of indexes may also return objects which do not in fact
	 * intersect the query envelope.
	 *
	 * @param searchEnv the envelope to query for
	 * @return a list of the items found by the query
	 */
	virtual vector<void*>* query(const Envelope *searchEnv)=0;

};

} // namespace geos

#endif

