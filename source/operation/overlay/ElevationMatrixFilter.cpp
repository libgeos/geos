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

#include <geos/geom.h>
#include <geos/opOverlay.h>

#define DEBUG 0

namespace geos
{

ElevationMatrixFilter::ElevationMatrixFilter(const ElevationMatrix *newEm):
	em(newEm), avgElevation(newEm->getAvgElevation())
{
}

ElevationMatrixFilter::~ElevationMatrixFilter()
{
}

void
ElevationMatrixFilter::filter_rw(Coordinate *c)
{
#if DEBUG
	cerr<<"ElevationMatrixFilter::filter_rw("<<c->toString()<<") called"
		<<endl;
#endif
	if ( c->z == DoubleNotANumber && avgElevation != DoubleNotANumber )
	{
		try {
			const ElevationMatrixCell &emc = em->getCell(*c);
			c->z = emc.getAvg();
			if ( c->z == DoubleNotANumber ) c->z = avgElevation;
#if DEBUG
			cerr<<"  z set to "<<c->z<<endl;
#endif
		} catch (IllegalArgumentException *ex) {
			delete ex;
			c->z = avgElevation;
		}
	}
#if DEBUG
	else
	{
		cerr<<"  already has a Z or avgElevation == NaN"<<endl;
	}
#endif
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 **********************************************************************/
