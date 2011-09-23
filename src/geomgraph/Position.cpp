/**********************************************************************
 * $Id: Position.cpp 2545 2009-06-05 12:07:52Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/Position.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geomgraph/Position.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
* Returns LEFT if the position is RIGHT, RIGHT if the position is LEFT, or the position
* otherwise.
*/
int Position::opposite(int position){
	if (position==LEFT) return RIGHT;
	if (position==RIGHT) return LEFT;
#if GEOS_DEBUG
	std::cerr<<"Position::opposite: position is neither LEFT ("<<LEFT<<") nor RIGHT ("<<RIGHT<<") but "<<position<<std::endl;
#endif
	return position;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.6  2006/03/09 16:01:59  strk
 * Fixed debugging lines
 *
 * Revision 1.5  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.4  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.3  2005/05/20 16:15:41  strk
 * Code cleanups
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

