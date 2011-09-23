/**********************************************************************
 * $Id: Depth.cpp 2544 2009-06-05 11:03:53Z strk $
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
 * Last port: geomgraph/Depth.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#include <sstream>
#include <string>

#include <geos/geomgraph/Depth.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Position.h>
#include <geos/geom/Location.h>

#define NULL_VALUE -1

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

int
Depth::depthAtLocation(int location)
{
	if (location == Location::EXTERIOR) return 0;
	if (location == Location::INTERIOR) return 1;
	return NULL_VALUE;
}

Depth::Depth()
{
	// initialize depth array to a sentinel value
	for (int i=0; i<2; i++) {
		for (int j=0; j<3;j++) {
			depth[i][j]=NULL_VALUE;
		}
	}
}

Depth::~Depth()
{
//	delete[] &depth;
}

int
Depth::getDepth(int geomIndex,int posIndex) const
{
	return depth[geomIndex][posIndex];
}

void
Depth::setDepth(int geomIndex,int posIndex,int depthValue)
{
	depth[geomIndex][posIndex] = depthValue;
}

int
Depth::getLocation(int geomIndex,int posIndex) const
{
	if (depth[geomIndex][posIndex] <= 0) return Location::EXTERIOR;
	return Location::INTERIOR;
}

void
Depth::add(int geomIndex,int posIndex,int location)
{
	if (location == Location::INTERIOR)
		depth[geomIndex][posIndex]++;
}

/**
 * A Depth object is null (has never been initialized) if all depths are null.
 */
bool
Depth::isNull() const
{
	for (int i=0; i<2; i++) {
		for (int j=0; j<3; j++) {
			if (depth[i][j] != NULL_VALUE)
				return false;
		}
	}
	return true;
}

bool
Depth::isNull(int geomIndex) const
{
	return depth[geomIndex][1] == NULL_VALUE;
}

bool
Depth::isNull(int geomIndex, int posIndex) const
{
	return depth[geomIndex][posIndex] == NULL_VALUE;
}

int
Depth::getDelta(int geomIndex) const
{
	return depth[geomIndex][Position::RIGHT]-depth[geomIndex][Position::LEFT];
}

/**
 * Normalize the depths for each geometry, if they are non-null.
 * A normalized depth
 * has depth values in the set { 0, 1 }.
 * Normalizing the depths
 * involves reducing the depths by the same amount so that at least
 * one of them is 0.  If the remaining value is > 0, it is set to 1.
 */
void
Depth::normalize()
{
	for (int i=0; i<2; i++) {
		if (!isNull(i)) {
			int minDepth=depth[i][1];
			if (depth[i][2]<minDepth)
				minDepth=depth[i][2];
			if (minDepth<0) minDepth = 0;
			for (int j=1; j<3; j++) {
				int newValue=0;
				if (depth[i][j]>minDepth)
					newValue = 1;
				depth[i][j] = newValue;
			}
		}
	}
}

void
Depth::add(const Label& lbl)
{
	for (int i=0; i<2; i++) {
		for (int j=1; j<3; j++) {
			int loc=lbl.getLocation(i,j);
			if (loc==Location::EXTERIOR || loc==Location::INTERIOR)
			{
				// initialize depth if it is null, otherwise
				// add this location value
				if (isNull(i,j)) {
					depth[i][j]=depthAtLocation(loc);
				} else
					depth[i][j]+=depthAtLocation(loc);
			}
		}
	}
}

string
Depth::toString() const
{
	ostringstream s;
	s<<"A:"<<depth[0][1]<<","<<depth[0][2]<<" ";
	s<<"B:"<<depth[1][1]<<","<<depth[1][2]<<"]";
	return s.str();
}


} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.6  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2005/11/14 18:14:03  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.3  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

