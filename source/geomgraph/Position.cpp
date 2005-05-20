/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/geomgraph.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

/**
* Returns LEFT if the position is RIGHT, RIGHT if the position is LEFT, or the position
* otherwise.
*/
int Position::opposite(int position){
	if (position==LEFT) return RIGHT;
	if (position==RIGHT) return LEFT;
#if DEBUG
	cerr<<"Position::opposite: position is neither LEFT ("<<LEFT<<") nor RIGHT ("<<RIGHT<<") but "<<position<<endl;
#endif
	return position;
}

} // namespace geos

/**********************************************************************
 * $Log$
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

