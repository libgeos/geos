/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/DouglasPeuckerLineSimplifier.java rev. 1.4
 *
 **********************************************************************/

#include <geos/simplify/DouglasPeuckerLineSimplifier.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>

#include <vector>
#include <memory> // for auto_ptr

namespace geos {

/// Line simplification algorithms
namespace simplify { // geos::simplify

/*public static*/
DouglasPeuckerLineSimplifier::CoordsVectAutoPtr
DouglasPeuckerLineSimplifier::simplify(
		const DouglasPeuckerLineSimplifier::CoordsVect& nPts,
		double distanceTolerance)
{
	DouglasPeuckerLineSimplifier simp(nPts);
	simp.setDistanceTolerance(distanceTolerance);
	return simp.simplify();
}

/*public*/
DouglasPeuckerLineSimplifier::DouglasPeuckerLineSimplifier(
		const DouglasPeuckerLineSimplifier::CoordsVect& nPts)
	:
	pts(nPts)
{
}

/*public*/
void
DouglasPeuckerLineSimplifier::setDistanceTolerance(
		double nDistanceTolerance)
{
	distanceTolerance=nDistanceTolerance;
}

/*public*/
DouglasPeuckerLineSimplifier::CoordsVectAutoPtr
DouglasPeuckerLineSimplifier::simplify()
{
	usePt = BoolVectAutoPtr(new BoolVect(pts.size(), true));
	simplifySection(0, pts.size() - 1);

	CoordsVectAutoPtr coordList(new CoordsVect());
	for (unsigned int i=0, n=pts.size(); i<n; ++i)
	{
		if ( usePt->operator[](i) )
		{
			coordList->push_back(pts[i]);
		}
	}

	// auto_ptr transfer ownership to its
	// returned copy
	return coordList;
}

/*private*/
void
DouglasPeuckerLineSimplifier::simplifySection(
		unsigned int i,
		unsigned int j)
{
	if ( (i+1) == j ) return;

	geom::LineSegment seg(pts[i], pts[j]);
	double maxDistance = -1.0;

	unsigned int maxIndex = i;

	for (unsigned int k=i+1; k<j; k++)
	{
		double distance = seg.distance(pts[k]);
		if (distance > maxDistance) {
			maxDistance = distance;
			maxIndex = k;
		}
	}
	if (maxDistance <= distanceTolerance) {
		for(unsigned int k =i+1; k<j; k++)
		{
			usePt->operator[](k) = false;
		}
	}
	else {
		simplifySection(i, maxIndex);
		simplifySection(maxIndex, j);
	}
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/03 10:16:11  strk
 * DouglasPeuckerLineSimplifier class port
 *
 **********************************************************************/
