/**********************************************************************
 * $Id: indexSweepline.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_INDEXSWEEPLINE_H
#define GEOS_INDEXSWEEPLINE_H

namespace geos {
namespace index { // geos.index

/// \brief
/// Contains classes which implement a sweepline algorithm
/// for scanning geometric data structures.
///
namespace sweepline { // geos.index.sweepline

} // namespace geos.index.sweepline
} // namespace geos.index
} // namespace geos

//#include <geos/index/sweepline/SweepLineEvent.h>
//#include <geos/index/sweepline/SweepLineIndex.h>
//#include <geos/index/sweepline/SweepLineInterval.h>
#include <geos/index/sweepline/SweepLineOverlapAction.h>

#endif

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/06/05 19:42:33  strk
 * Fixed noinst_geos_HEADERS => noinst_HEADERS.
 *
 * Revision 1.7  2006/06/01 10:28:46  strk
 * Reduced number of installed headers for the geos::index namespace
 *
 * Revision 1.6  2006/03/21 10:01:30  strk
 * indexSweepline.h header split
 *
 **********************************************************************/

