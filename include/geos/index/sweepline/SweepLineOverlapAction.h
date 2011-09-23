/**********************************************************************
 * $Id: SweepLineOverlapAction.h 2556 2009-06-06 22:22:28Z strk $
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
 **********************************************************************/

#ifndef GEOS_INDEX_SWEEPLINE_SWEEPLINEOVERLAPACTION_H
#define GEOS_INDEX_SWEEPLINE_SWEEPLINEOVERLAPACTION_H

#include <geos/export.h>

// Forward declarations
namespace geos {
	namespace index { 
		namespace sweepline {
			class SweepLineInterval;
		}
	}
}

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos:index:sweepline

class GEOS_DLL SweepLineOverlapAction {
public:
	virtual void overlap(SweepLineInterval *s0,SweepLineInterval *s1)=0;

	virtual ~SweepLineOverlapAction() {}
};


} // namespace geos:index:sweepline
} // namespace geos:index
} // namespace geos

#endif // GEOS_INDEX_SWEEPLINE_SWEEPLINEOVERLAPACTION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.1  2006/03/21 10:01:30  strk
 * indexSweepline.h header split
 *
 **********************************************************************/

