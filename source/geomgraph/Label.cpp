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

#include <string>

#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/TopologyLocation.h>
#include <geos/geomgraph/Position.h>
#include <geos/geom/Location.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

Label*
Label::toLineLabel(const Label &label)
{
	Label *lineLabel=new Label(Location::UNDEF);
	for (int i=0; i<2; i++) {
		lineLabel->setLocation(i, label.getLocation(i));
	}
	return lineLabel;
}

/**
 * Construct a Label with a single location for both Geometries.
 */
Label::Label(int onLoc)
{
	elt[0]=TopologyLocation(onLoc);
	elt[1]=TopologyLocation(onLoc);
}

/**
 * Construct a Label with a single location for both Geometries.
 * Initialize the location for the Geometry index.
 */
Label::Label(int geomIndex,int onLoc)
{
	elt[0]=TopologyLocation(Location::UNDEF);
	elt[1]=TopologyLocation(Location::UNDEF);
	elt[geomIndex].setLocation(onLoc);
}

/**
 * Construct a Label with On, Left and Right locations for both Geometries.
 * Initialize the locations for both Geometries to the given values.
 */
Label::Label(int onLoc,int leftLoc,int rightLoc)
{
	elt[0]=TopologyLocation(onLoc,leftLoc,rightLoc);
	elt[1]=TopologyLocation(onLoc,leftLoc,rightLoc);
}

/*
 * Initialize the locations to Null
 */
Label::Label()
{
	elt[0]=TopologyLocation(Location::UNDEF);
	elt[1]=TopologyLocation(Location::UNDEF);
}

Label::Label(const Label &l)
{
	elt[0]=TopologyLocation(l.elt[0]);
	elt[1]=TopologyLocation(l.elt[1]);
}

Label::~Label()
{
	// STOP IT
}

/**
 * Construct a Label with On, Left and Right locations for both Geometries.
 * Initialize the locations for the given Geometry index.
 */
Label::Label(int geomIndex,int onLoc,int leftLoc,int rightLoc)
{
	elt[0]=TopologyLocation(Location::UNDEF,Location::UNDEF,Location::UNDEF);
	elt[1]=TopologyLocation(Location::UNDEF,Location::UNDEF,Location::UNDEF);
	elt[geomIndex].setLocations(onLoc,leftLoc,rightLoc);
}

void
Label::flip()
{
	elt[0].flip();
	elt[1].flip();
}

int
Label::getLocation(int geomIndex,int posIndex) const
{
	return elt[geomIndex].get(posIndex);
}

int
Label::getLocation(int geomIndex) const
{
	return elt[geomIndex].get(Position::ON);
}

void
Label::setLocation(int geomIndex,int posIndex,int location)
{
	elt[geomIndex].setLocation(posIndex,location);
}

void
Label::setLocation(int geomIndex,int location)
{
	elt[geomIndex].setLocation(Position::ON,location);
}

void
Label::setAllLocations(int geomIndex,int location)
{
	elt[geomIndex].setAllLocations(location);
}

void
Label::setAllLocationsIfNull(int geomIndex,int location)
{
	elt[geomIndex].setAllLocationsIfNull(location);
}

void
Label::setAllLocationsIfNull(int location)
{
	setAllLocationsIfNull(0,location);
	setAllLocationsIfNull(1,location);
}

/**
 * Merge this label with another one.
 * Merging updates any null attributes of this label with the attributes
 * from lbl
 */
void
Label::merge(const Label &lbl)
{
	for (int i=0; i<2; i++) {
		elt[i].merge(lbl.elt[i]);
	}
}

int
Label::getGeometryCount() const
{
	int count = 0;
	if (!elt[0].isNull()) count++;
	if (!elt[1].isNull()) count++;
	return count;
}

bool
Label::isNull(int geomIndex) const
{
	return elt[geomIndex].isNull();
}

bool
Label::isAnyNull(int geomIndex) const
{
	return elt[geomIndex].isAnyNull();
}

bool
Label::isArea() const
{
	return elt[0].isArea() || elt[1].isArea();
}

bool
Label::isArea(int geomIndex) const
{
	return elt[geomIndex].isArea();
}

bool
Label::isLine(int geomIndex) const
{
	return elt[geomIndex].isLine();
}

bool
Label::isEqualOnSide(const Label& lbl, int side) const
{
	return 
		elt[0].isEqualOnSide(lbl.elt[0], side)
		&& elt[1].isEqualOnSide(lbl.elt[1], side);
}

bool
Label::allPositionsEqual(int geomIndex, int loc) const
{
	return elt[geomIndex].allPositionsEqual(loc);
}

/**
 * Converts one GeometryLocation to a Line location
 */
void
Label::toLine(int geomIndex)
{
	if (elt[geomIndex].isArea()) {
		elt[geomIndex]=TopologyLocation(elt[geomIndex].getLocations()[0]);
	}
}

string
Label::toString() const
{
	string buf="";
	buf.append("a:");
	buf.append(elt[0].toString());
	buf.append(" b:");
	buf.append(elt[1].toString());
	return buf;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/04/03 17:05:22  strk
 * Assertion checking, port info, cleanups
 *
 * Revision 1.8  2006/03/14 12:55:55  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.7  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 **********************************************************************/

