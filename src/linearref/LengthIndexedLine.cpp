/**********************************************************************
 * $Id: LengthIndexedLine.cpp 2809 2009-12-06 01:05:24Z mloskot $
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
 * Last port: linearref/LengthIndexedLine.java rev. 1.35
 *
 **********************************************************************/

#include <geos/linearref/ExtractLineByLocation.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LengthLocationMap.h>
#include <geos/linearref/LengthIndexOfPoint.h>
#include <geos/linearref/LocationIndexOfLine.h>

using namespace std;

using namespace geos::geom;

namespace geos
{
namespace linearref   // geos.linearref
{

LengthIndexedLine::LengthIndexedLine(const Geometry* linearGeom) :
		linearGeom(linearGeom) {}

Coordinate LengthIndexedLine::extractPoint(double index) const
{
	LinearLocation loc = LengthLocationMap::getLocation(linearGeom, index);
	Coordinate coord = loc.getCoordinate(linearGeom);
	return coord;
}

Coordinate LengthIndexedLine::extractPoint(double index, double offsetDistance) const
{
	LinearLocation loc = LengthLocationMap::getLocation(linearGeom, index);
	Coordinate ret;
	loc.getSegment(linearGeom)->pointAlongOffset(loc.getSegmentFraction(), offsetDistance, ret);
	return ret;
}


Geometry *LengthIndexedLine::extractLine(double startIndex, double endIndex) const
{

	const LinearLocation startLoc = locationOf(startIndex);
	const LinearLocation endLoc = locationOf(endIndex);
	Geometry* g = ExtractLineByLocation::extract(linearGeom, startLoc, endLoc);
	return g;
}

LinearLocation LengthIndexedLine::locationOf(double index) const
{
	return LengthLocationMap::getLocation(linearGeom, index);
}


double LengthIndexedLine::indexOf(const Coordinate& pt) const
{
	return LengthIndexOfPoint::indexOf(linearGeom, pt);
}


double LengthIndexedLine::indexOfAfter(const Coordinate& pt, double minIndex) const
{
	return LengthIndexOfPoint::indexOfAfter(linearGeom, pt, minIndex);
}


double* LengthIndexedLine::indicesOf(const Geometry* subLine) const
{
	LinearLocation* locIndex = LocationIndexOfLine::indicesOf(linearGeom, subLine);
	double* index = new double[2];
	index[0] = LengthLocationMap::getLength(linearGeom, locIndex[0]);
	index[1] = LengthLocationMap::getLength(linearGeom, locIndex[1]);
	delete [] locIndex;
	return index;
}


double LengthIndexedLine::project(const Coordinate& pt) const
{
	return LengthIndexOfPoint::indexOf(linearGeom, pt);
}

double LengthIndexedLine::getStartIndex() const
{
	return 0.0;
}

double LengthIndexedLine::getEndIndex() const
{
	return linearGeom->getLength();
}

bool LengthIndexedLine::isValidIndex(double index) const
{
	return (index >= getStartIndex()
		&& index <= getEndIndex());
}

double LengthIndexedLine::clampIndex(double index) const
{
	double startIndex = getStartIndex();
	if (index < startIndex) return startIndex;

	double endIndex = getEndIndex();
	if (index > endIndex) return endIndex;

	return index;
}
}
}
