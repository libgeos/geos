/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/opOverlay.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;
using namespace std; // for GEOS_DEBUG

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

ElevationMatrixFilter::ElevationMatrixFilter(ElevationMatrix &newEm):
	em(newEm)
{
}

ElevationMatrixFilter::~ElevationMatrixFilter()
{
}

void
ElevationMatrixFilter::filter_rw(Coordinate *c) const
{
#if GEOS_DEBUG
	cerr<<"ElevationMatrixFilter::filter_rw("<<c->toString()<<") called"
		<<endl;
#endif

	// already has a Z value, nothing to do
	if ( ! ISNAN(c->z) ) return;

	double avgElevation = em.getAvgElevation();

	try {
		const ElevationMatrixCell &emc = em.getCell(*c);
		c->z = emc.getAvg();
		if ( ISNAN(c->z) ) c->z = avgElevation;
#if GEOS_DEBUG
		cerr<<"  z set to "<<c->z<<endl;
#endif
	} catch (const util::IllegalArgumentException& /* ex */) {
		c->z = avgElevation;
	}
}

void
ElevationMatrixFilter::filter_ro(const Coordinate *c)
{
#if GEOS_DEBUG
	cerr<<"ElevationMatrixFilter::filter_ro("<<c->toString()<<") called"
		<<endl;
#endif
	em.add(*c);
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.12  2006/03/10 10:44:53  strk
 * Unreferenced exception objects cleanup (#52)
 *
 * Revision 1.11  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.10  2006/03/09 15:58:43  strk
 * Fixed debugging lines
 *
 * Revision 1.9  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.8  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.7  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.6  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.5  2005/12/11 10:41:57  strk
 * Fixed premature initialization of average Z value in ElevationMatrixFilter
 *
 * Revision 1.4  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
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
