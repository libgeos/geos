/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTINTERSECTOR_H
#define GEOS_NODING_SEGMENTINTERSECTOR_H

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace noding {
		class SegmentString;
	}
}

namespace geos {
namespace noding { // geos.noding


/**
 * \brief
 * Computes the intersections between two line segments in SegmentString
 * and adds them to each string.
 *
 * The SegmentIntersector is passed to a Noder.
 * The addIntersections method is called whenever the Noder
 * detects that two SegmentStrings <i>might</i> intersect.
 * This class is an example of the <i>Strategy</i> pattern.
 *
 * Last port: noding/SegmentIntersector.java rev. 1.7 (JTS-1.7)
 *
 */
class SegmentIntersector {

public:

	/**
	 * This method is called by clients
	 * of the {@link SegmentIntersector} interface to process
	 * intersections for two segments of the {@link SegmentStrings}
	 * being intersected.
	 */
	virtual void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1)=0;

protected:

	SegmentIntersector() {}
	virtual ~SegmentIntersector() {}
 
};

/// Temporary typedef for namespace transition
typedef SegmentIntersector nodingSegmentIntersector;

} // namespace geos.noding
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/noding/SegmentIntersector.inl"
//#endif

#endif // GEOS_NODING_SEGMENTINTERSECTOR_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

