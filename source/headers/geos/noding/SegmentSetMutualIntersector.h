/**********************************************************************
 * $Id
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTSETMUTUALINTERSECTOR_H
#define GEOS_NODING_SEGMENTSETMUTUALINTERSECTOR_H

#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>

namespace geos {
namespace noding { // geos::noding

class SegmentSetMutualIntersector
{
private:
protected:
	SegmentIntersector * segInt;

public:
	SegmentSetMutualIntersector() 
	:	segInt( NULL)
	{ }

	virtual ~SegmentSetMutualIntersector() 
	{ }

	/**
	 * Sets the {@link SegmentIntersector} to use with this intersector.
	 * The SegmentIntersector will either rocord or add intersection nodes
	 * for the input segment strings.
	 *
	 * @param segInt the segment intersector to use
	 */
	void setSegmentIntersector( SegmentIntersector * si)
	{
		segInt = si;
	}

	/**
	 * 
	 * @param segStrings0 a collection of {@link SegmentString}s to node
	 */
	virtual void setBaseSegments( SegmentString::ConstVect * segStrings) =0; 

	/**
	 * Computes the intersections for two collections of {@link SegmentString}s.
	 *
	 * @param segStrings1 a collection of {@link SegmentString}s to node
	 */
	virtual void process( SegmentString::ConstVect * segStrings) =0;

};

} // geos::noding
} // geos

#endif // GEOS_NODING_SEGMENTSETMUTUALINTERSECTOR_H
/**********************************************************************
 * $Log$
 **********************************************************************/

