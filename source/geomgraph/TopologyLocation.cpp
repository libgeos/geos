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
#include <vector>

using namespace std;

namespace geos {
namespace geomgraph { // geos.geomgraph

TopologyLocation::TopologyLocation(const vector<int> &newLocation):
	location(newLocation.size(), Location::UNDEF)
{
}

TopologyLocation::TopologyLocation()
{
}

TopologyLocation::~TopologyLocation()
{
}

/**
 * Constructs a TopologyLocation specifying how points on, to the
 * left of, and to the right of some GraphComponent relate to some
 * Geometry. Possible values for the
 * parameters are Location.NULL, Location.EXTERIOR, Location.BOUNDARY, 
 * and Location.INTERIOR.
 * @see Location
 */
TopologyLocation::TopologyLocation(int on, int left, int right):
	location(3)
{
	location[Position::ON]=on;
	location[Position::LEFT]=left;
	location[Position::RIGHT]=right;
}

TopologyLocation::TopologyLocation(int on):
	location(1, on)
{
	//(*location)[Position::ON]=on;
}

TopologyLocation::TopologyLocation(const TopologyLocation &gl):
	location(gl.location)
{
	//location=new vector<int>(gl->location->begin(),gl->location->end());
}

int
TopologyLocation::get(unsigned int posIndex) const
{
	if (posIndex<location.size()) return location[posIndex];
	return Location::UNDEF;
}

/**
 * @return true if all locations are NULL
 */
bool
TopologyLocation::isNull() const
{
	for (unsigned int i=0, sz=location.size(); i<sz; ++i) {
		if (location[i]!=Location::UNDEF) return false;
	}
	return true;
}

/**
 * @return true if any locations are NULL
 */
bool
TopologyLocation::isAnyNull() const
{
	for (unsigned int i=0, sz=location.size(); i<sz; ++i) {
		if (location[i]==Location::UNDEF) return true;
	}
	return false;
}

bool
TopologyLocation::isEqualOnSide(const TopologyLocation &le, int locIndex) const
{
	return location[locIndex]==le.location[locIndex];
}

bool
TopologyLocation::isArea() const
{
	return location.size()>1;
}

bool
TopologyLocation::isLine() const
{
	return location.size()==1;
}

void
TopologyLocation::flip()
{
	if (location.size()<=1) return;
	int temp=location[Position::LEFT];
	location[Position::LEFT]=location[Position::RIGHT];
	location[Position::RIGHT] = temp;
}

void
TopologyLocation::setAllLocations(int locValue)
{
	for (unsigned int i=0, sz=location.size(); i<sz; ++i) {
		location[i]=locValue;
	}
}

void
TopologyLocation::setAllLocationsIfNull(int locValue)
{
	for (unsigned int i=0, sz=location.size(); i<sz; ++i) {
		if (location[i]==Location::UNDEF) location[i]=locValue;
	}
}

void
TopologyLocation::setLocation(unsigned int locIndex, int locValue)
{
	location[locIndex]=locValue;
}

void
TopologyLocation::setLocation(int locValue)
{
	setLocation(Position::ON, locValue);
}

const vector<int> &
TopologyLocation::getLocations() const
{
	return location;
}

void
TopologyLocation::setLocations(int on, int left, int right)
{
	location[Position::ON]=on;
	location[Position::LEFT]=left;
	location[Position::RIGHT]=right;
}

bool
TopologyLocation::allPositionsEqual(int loc) const
{
	for (unsigned int i=0, sz=location.size(); i<sz; ++i) {
		if (location[i]!=loc) return false;
	}
	return true;
}

/**
 * merge updates only the NULL attributes of this object
 * with the attributes of another.
 */
void
TopologyLocation::merge(const TopologyLocation &gl)
{
	// if the src is an Area label & and the dest is not, increase the dest to be an Area
	unsigned int sz=location.size();
	unsigned int glsz=gl.location.size();
	if (glsz>sz) {
		location.resize(3);
		location[Position::LEFT]=Location::UNDEF;
		location[Position::RIGHT]=Location::UNDEF;
	}
	for (unsigned int i=0; i<sz; ++i) {
		if (location[i]==Location::UNDEF && i<glsz)
			location[i]=gl.location[i];
	}
}

string
TopologyLocation::toString() const
{
	string buf="";
	if (location.size()>1) buf+=Location::toLocationSymbol(location[Position::LEFT]);
	buf+=Location::toLocationSymbol(location[Position::ON]);
	if (location.size()>1) buf+=Location::toLocationSymbol(location[Position::RIGHT]);
	return buf;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2005/11/29 15:45:39  strk
 * Fixed signedness of TopologyLocation methods, cleanups.
 *
 * Revision 1.4  2005/11/15 18:30:59  strk
 * Removed dead code
 *
 * Revision 1.3  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

