/**********************************************************************
 * $Id: Interval.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/index/strtree/Interval.h>
//#include <geos/util.h>

#include <algorithm>
#include <typeinfo>
#include <cassert>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree

Interval::Interval(Interval *other)
{
	Interval(other->imin,other->imax);
}

Interval::Interval(double newMin,double newMax)
{
	assert(newMin<=newMax);
	imin=newMin;
	imax=newMax;
}

double
Interval::getCentre()
{
	return (imin+imax)/2;
}

Interval*
Interval::expandToInclude(Interval *other)
{
	imax=max(imax,other->imax);
	imin=min(imin,other->imin);
	return this;
}

bool
Interval::intersects(Interval *other)
{
	return !(other->imin>imax || other->imax<imin);
}

bool
Interval::equals(void *o) {
	if (typeid(o)!=typeid(Interval)) {
		return false;
	}
	Interval *other=(Interval*) o;
	return imin==other->imin && imax==other->imax;
}

} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 * Revision 1.10  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.9  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.7  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

