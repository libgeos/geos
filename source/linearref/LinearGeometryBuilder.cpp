/**********************************************************************
 * $Id: ExtractLineByLocation.cpp 1938 2006-12-07 10:45:16Z strk $
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
 * Last port: linearref/ExtractLineByLocation.java rev. 1.35
 *
 **********************************************************************/

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/LineString.h>
#include <geos/linearref/ExtractLineByLocation.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/linearref/LinearLocation.h>
#include <geos/linearref/LengthLocationMap.h>
#include <geos/linearref/LengthIndexOfPoint.h>
#include <geos/linearref/LinearGeometryBuilder.h>
#include <geos/util/IllegalArgumentException.h>

using namespace std;

using namespace geos::geom;

namespace geos
{
namespace linearref   // geos.linearref
{

LinearGeometryBuilder::LinearGeometryBuilder(const GeometryFactory* geomFact) :
		geomFact(geomFact),
		ignoreInvalidLines(false),
		fixInvalidLines(false),
		coordList(0) {}

void LinearGeometryBuilder::setIgnoreInvalidLines(bool ignoreInvalidLines)
{
	this->ignoreInvalidLines = ignoreInvalidLines;
}

/**
 * Allows invalid lines to be ignored rather than causing Exceptions.
 * An invalid line is one which has only one unique point.
 *
 * @param ignoreShortLines <code>true</code> if short lines are to be ignored
 */
void LinearGeometryBuilder::setFixInvalidLines(bool fixInvalidLines)
{
	this->fixInvalidLines = fixInvalidLines;
}

/**
 * Adds a point to the current line.
 *
 * @param pt the Coordinate to add
 */
void LinearGeometryBuilder::add(const Coordinate& pt)
{
	add(pt, true);
}

/**
 * Adds a point to the current line.
 *
 * @param pt the Coordinate to add
 */
void LinearGeometryBuilder::add(const Coordinate& pt, bool allowRepeatedPoints)
{
	if (!coordList)
		coordList = new CoordinateArraySequence();
	coordList->add(pt, allowRepeatedPoints);
	lastPt = pt;
}

Coordinate LinearGeometryBuilder::getLastCoordinate() const
{
	return lastPt;
}

/**
 * Terminate the current LineString.
 */
void LinearGeometryBuilder::endLine()
{
	if (!coordList)
	{
		return;
	}
	if (coordList->size() < 2)
	{
		if (ignoreInvalidLines)
		{
			if (coordList)
			{
				delete coordList;
				coordList = 0;
			}
			return;
		}
		else if (fixInvalidLines)
		{
			add((*coordList)[0]);
		}
	}

	LineString* line = 0;
	try
	{
		line = geomFact->createLineString(coordList);
	}
	catch (util::IllegalArgumentException ex)
	{
		// exception is due to too few points in line.
		// only propagate if not ignoring short lines
		if (! ignoreInvalidLines)
			throw ex;
	}

	if (line) lines.push_back(line);
	coordList = 0;
}

Geometry* LinearGeometryBuilder::getGeometry()
{
	// end last line in case it was not done by user
	endLine();
	return geomFact->buildGeometry(lines);
}
}
}
