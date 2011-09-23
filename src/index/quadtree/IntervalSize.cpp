/**********************************************************************
 * $Id: IntervalSize.cpp 2411 2009-04-27 15:39:42Z strk $
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
 **********************************************************************
 *
 * Last port: index/quadtree/IntervalSize.java rev 1.7 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/quadtree/IntervalSize.h>
#include <geos/index/quadtree/DoubleBits.h>

#include <algorithm>
#include <cmath>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

/* public static */
bool
IntervalSize::isZeroWidth(double mn, double mx)
{
	double width = mx - mn;
	if (width == 0.0) return true;

	double maxAbs = max(fabs(mn), fabs(mx));
	double scaledInterval = width / maxAbs;
	int level = DoubleBits::exponent(scaledInterval);
	return level <= MIN_BINARY_EXPONENT;
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 * Revision 1.9  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.8  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.7  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

