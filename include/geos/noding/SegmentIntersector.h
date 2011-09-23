/**********************************************************************
 * $Id: SegmentIntersector.h 2556 2009-06-06 22:22:28Z strk $
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

#include <geos/export.h>

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
 * Processes possible intersections detected by a Noder.
 *
 * The SegmentIntersector is passed to a Noder.
 * The addIntersections method is called whenever the Noder
 * detects that two SegmentStrings <i>might</i> intersect.
 * This class may be used either to find all intersections, or
 * to detect the presence of an intersection.  In the latter case,
 * Noders may choose to short-circuit their computation by calling the
 * isDone method.
 * This class is an example of the <i>Strategy</i> pattern.
 *
 * @version 1.7
 */
class GEOS_DLL SegmentIntersector {

public:

	/**
	 * This method is called by clients
	 * of the SegmentIntersector interface to process
	 * intersections for two segments of the SegmentStrings
	 * being intersected.
	 */
	virtual void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1)=0;

	/**
	 * \brief
	 * Reports whether the client of this class
	 * needs to continue testing all intersections in an arrangement.
	 * 
	 * @return true if there is not need to continue testing segments
	 *
	 * The default implementation always return false (process all intersections).
	 */
	virtual bool isDone() const {
		return false;
	}

	virtual ~SegmentIntersector() 
	{ }

protected:

	SegmentIntersector() {}
 
};

/// Temporary typedef for namespace transition
typedef SegmentIntersector nodingSegmentIntersector;

} // namespace geos.noding
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/noding/SegmentIntersector.inl"
//#endif

#endif // GEOS_NODING_SEGMENTINTERSECTOR_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/


