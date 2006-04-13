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
 * Last port: simplify/TaggedLineString.java rev. 1.2 (JTS-1.7.1)
 *
 **********************************************************************/

#include <geos/simplify/TaggedLineString.h>
#include <geos/simplify/TaggedLineSegment.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Geometry.h> // for auto_ptr destructor 
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>

#include <cassert>
#include <memory>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;
using namespace std;

namespace geos {
namespace simplify { // geos::simplify

/*public*/
TaggedLineString::TaggedLineString(const geom::LineString* nParentLine,
			unsigned int nMinimumSize)
	:
	parentLine(nParentLine),
	minimumSize(nMinimumSize)
{
	init();
}

/*public*/
TaggedLineString::~TaggedLineString()
{
	for (unsigned int i=0, n=segs.size(); i<n; i++)
		delete segs[i];

	for (unsigned int i=0, n=resultSegs.size(); i<n; i++)
		delete resultSegs[i];
}

/*private*/
void
TaggedLineString::init()
{
	const CoordinateSequence* pts = parentLine->getCoordinatesRO();
	segs.reserve(pts->size()-1);

	for (unsigned int i=0, n=pts->size()-1; i<n; i++)
	{
		TaggedLineSegment* seg = new TaggedLineSegment(
				pts->getAt(i),
				pts->getAt(i+1),
				parentLine, i);

		segs.push_back(seg);
	}
}

/*public*/
unsigned int
TaggedLineString::getMinimumSize() const
{
	return minimumSize;
}

/*public*/
const geom::LineString* 
TaggedLineString::getParent() const
{
	return parentLine;
}

/*public*/
const CoordinateSequence*
TaggedLineString::getParentCoordinates() const
{
	return parentLine->getCoordinatesRO();
}

/*public*/
CoordinateSequence::AutoPtr
TaggedLineString::getResultCoordinates() const
{
	CoordVectPtr pts = extractCoordinates(resultSegs);
	CoordVect* v = pts.release();
	return CoordinateSequence::AutoPtr(parentLine->getFactory()->getCoordinateSequenceFactory()->create(v));
}

/*private static*/
TaggedLineString::CoordVectPtr
TaggedLineString::extractCoordinates(
		const std::vector<TaggedLineSegment*>& segs)
{
	CoordVectPtr pts(new CoordVect());

	unsigned int i, n;

	for (i=0, n=segs.size(); i<n; i++)
	{
		TaggedLineSegment* seg = segs[i];
		pts->push_back(seg->p0);
	}

	// add last point
	pts->push_back(segs[i]->p1);

	return pts;
}

/*public*/
unsigned int
TaggedLineString::getResultSize() const
{
	unsigned resultSegsSize = resultSegs.size();
	return resultSegsSize == 0 ? 0 : resultSegsSize + 1;
}

/*public*/
TaggedLineSegment*
TaggedLineString::getSegment(unsigned int i) 
{
	return segs[i];
}

/*public*/
vector<TaggedLineSegment*>&
TaggedLineString::getSegments()
{
	return segs;
}

/*public*/
const vector<TaggedLineSegment*>&
TaggedLineString::getSegments() const
{
	return segs;
}

/*public*/
auto_ptr<Geometry>
TaggedLineString::asLineString() const
{
	return parentLine->getFactory()->createLineString(
			getResultCoordinates());
}

/*public*/
auto_ptr<Geometry>
TaggedLineString::asLinearRing() const
{
	return parentLine->getFactory()->createLinearRing(
			getResultCoordinates());
}

/*public*/
void
TaggedLineString::addToResult(auto_ptr<TaggedLineSegment> seg)
{
	segs.push_back(seg.release());
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/04/13 09:21:46  mloskot
 * Removed definition of copy ctor and assignment operator for TaggedLineString class.
 * According to following rule: Declaring, but not defining, private copy operations has
 * the effect of "turning off" copying for the class.
 *
 * Revision 1.2  2006/04/12 15:20:37  strk
 * LineSegmentIndex class
 *
 * Revision 1.1  2006/04/12 14:22:12  strk
 * Initial implementation of TaggedLineSegment and TaggedLineString classes
 *
 **********************************************************************/
