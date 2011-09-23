/**********************************************************************
 * $Id: LengthLocationMap.cpp 2809 2009-12-06 01:05:24Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/LengthLocationMap.java rev. 1.35
 *
 **********************************************************************/


#include <geos/linearref/LengthIndexedLine.h>
#include <geos/linearref/LinearIterator.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LengthLocationMap.h>

using namespace std;

using namespace geos::geom;

namespace geos
{
namespace linearref   // geos.linearref
{


LinearLocation LengthLocationMap::getLocation(const Geometry* linearGeom, double length)
{
	LengthLocationMap locater(linearGeom);
	return locater.getLocation(length);
}

double LengthLocationMap::getLength(const Geometry* linearGeom, const LinearLocation& loc)
{
	LengthLocationMap locater(linearGeom);
	return locater.getLength(loc);
}


LengthLocationMap::LengthLocationMap(const Geometry* linearGeom) :
		linearGeom(linearGeom) {}

LinearLocation LengthLocationMap::getLocation(double length) const
{
	double forwardLength = length;
	if (length < 0.0)
	{
		double lineLen = linearGeom->getLength();
		forwardLength = lineLen + length;
	}
	return getLocationForward(forwardLength);
}

LinearLocation LengthLocationMap::getLocationForward(double length) const
{
	if (length <= 0.0)
		return LinearLocation();

	double totalLength = 0.0;

	LinearIterator it (linearGeom);
	while (it.hasNext())
	{
		if (! it.isEndOfLine())
		{
			Coordinate p0 = it.getSegmentStart();
			Coordinate p1 = it.getSegmentEnd();
			double segLen = p1.distance(p0);
			// length falls in this segment
			if (totalLength + segLen > length)
			{
				double frac = (length - totalLength) / segLen;
				unsigned int compIndex = it.getComponentIndex();
				unsigned int segIndex = it.getVertexIndex();
				return LinearLocation(compIndex, segIndex, frac);
			}
			totalLength += segLen;
		}
		it.next();
	}
	// length is longer than line - return end location
	return LinearLocation::getEndLocation(linearGeom);
}


double LengthLocationMap::getLength(const LinearLocation& loc) const
{
	double totalLength = 0.0;

	LinearIterator it(linearGeom);
	while (it.hasNext())
	{
		if (! it.isEndOfLine())
		{
			Coordinate p0 = it.getSegmentStart();
			Coordinate p1 = it.getSegmentEnd();
			double segLen = p1.distance(p0);
			// length falls in this segment
			if (loc.getComponentIndex() == it.getComponentIndex()
					&& loc.getSegmentIndex() == it.getVertexIndex())
			{
				return totalLength + segLen * loc.getSegmentFraction();
			}
			totalLength += segLen;
		}
		it.next();
	}
	return totalLength;
}

}
}
