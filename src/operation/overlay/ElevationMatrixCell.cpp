/**********************************************************************
 * $Id: ElevationMatrixCell.cpp 2873 2010-01-18 18:15:40Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: original (by strk)
 *
 **********************************************************************/

#include <geos/platform.h>
#include <geos/geom/Coordinate.h>
#include <geos/operation/overlay/ElevationMatrixCell.h>

#include <sstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

ElevationMatrixCell::ElevationMatrixCell(): ztot(0)
{
}

ElevationMatrixCell::~ElevationMatrixCell()
{
}

void
ElevationMatrixCell::add(const Coordinate &c)
{
	if ( !ISNAN(c.z) )
	{
		if ( zvals.insert(c.z).second )
		{
			ztot+=c.z;
		}
	}
}

void
ElevationMatrixCell::add(double z)
{
	if ( !ISNAN(z) )
	{
		if ( zvals.insert(z).second )
		{
			ztot+=z;
		}
	}
}

double
ElevationMatrixCell::getTotal() const
{
	return ztot;
}

double
ElevationMatrixCell::getAvg() const
{
	if ( ! zvals.size() ) return DoubleNotANumber;
	return (ztot/zvals.size());
}

string
ElevationMatrixCell::print() const
{
	ostringstream ret;
	//ret<<"["<<ztot<<"/"<<zvals.size()<<"]";
	ret<<"["<<ztot/zvals.size()<<"]";
	return ret.str();
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos;

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.5  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.4  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.3  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.2  2004/11/26 09:22:50  strk
 * Added FINITE(x) macro and its use.
 * Made input geoms average Z computation optional in OverlayOp.
 *
 * Revision 1.1  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 **********************************************************************/
