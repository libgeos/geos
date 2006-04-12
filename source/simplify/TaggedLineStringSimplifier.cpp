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
 * Last port: simplify/TaggedLineStringSimplifier.java rev. 1.8 (JTS-1.7.1)
 *
 **********************************************************************/

#include <geos/simplify/TaggedLineStringSimplifier.h>
#include <geos/simplify/LineSegmentIndex.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/simplify/TaggedLineString.h>
#include <geos/simplify/TaggedLineSegment.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
//#include <geos/geom/Geometry.h> // for auto_ptr destructor 
//#include <geos/geom/GeometryFactory.h>
//#include <geos/geom/CoordinateSequenceFactory.h>

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
TaggedLineStringSimplifier::TaggedLineStringSimplifier(
		LineSegmentIndex* nInputIndex,
		LineSegmentIndex* nOutputIndex)
	:
	inputIndex(nInputIndex),
	outputIndex(nOutputIndex),
	li(new algorithm::LineIntersector()),
	line(NULL),
	linePts(NULL),
	distanceTolerance(0.0)
{
}

/*public*/
void
TaggedLineStringSimplifier::simplify(TaggedLineString* nLine)
{
	line = line;
	linePts = line->getParentCoordinates();
	simplifySection(0, linePts->size() - 1, 0);
}


/*private*/
void
TaggedLineStringSimplifier::simplifySection(unsigned int i,
		unsigned int j, unsigned int depth)
{
	depth += 1;

	vector<unsigned int> sectionIndex(2);

	if((i+1) == j)
	{
		auto_ptr<TaggedLineSegment> newSeg(new
			TaggedLineSegment(*(line->getSegment(i))));

		line->addToResult(newSeg);
		// leave this segment in the input index, for efficiency
		return;
	}

	bool isValidToSimplify = true;

	/**
	 * Following logic ensures that there is enough points in the
	 * output line.
	 * If there is already more points than the minimum, there's
	 * nothing to check.
	 * Otherwise, if in the worst case there wouldn't be enough points,
	 * don't flatten this segment (which avoids the worst case scenario)
	 */
	if (line->getResultSize() < line->getMinimumSize())
	{
		unsigned int worstCaseSize = depth + 1;
		if (worstCaseSize < line->getMinimumSize())
			isValidToSimplify = false;
	}

	double distance;

	// pass distance by ref
	int furthestPtIndex = findFarthestPoint(linePts, i, j, distance);

	// flattening must be less than distanceTolerance
	if ( distance > distanceTolerance ) isValidToSimplify = false;

	// test if flattened section would cause intersection
	LineSegment candidateSeg(linePts->getAt(i), linePts->getAt(j));

	sectionIndex[0] = i;
	sectionIndex[1] = j;

	if (hasBadIntersection(line, sectionIndex, candidateSeg))
			isValidToSimplify = false;

	if (isValidToSimplify) {
		auto_ptr<TaggedLineSegment> newSeg = flatten(i, j);
		line->addToResult(newSeg);
		return;
	}

	simplifySection(i, furthestPtIndex, depth);
	simplifySection(furthestPtIndex, j, depth);
 
}


/*private*/
auto_ptr<TaggedLineSegment>
TaggedLineStringSimplifier::flatten(unsigned int start, unsigned int end)
{
	// make a new segment for the simplified geometry
	const Coordinate& p0 = linePts->getAt(start);
	const Coordinate& p1 = linePts->getAt(end);
	auto_ptr<TaggedLineSegment> newSeg(new TaggedLineSegment(p0, p1));
	// update the indexes
	remove(line, start, end);
	outputIndex->add(newSeg.get());
	return newSeg;
}

/*private*/
bool
TaggedLineStringSimplifier::hasBadIntersection(
		const TaggedLineString* parentLine,
		const vector<unsigned int>& sectionIndex,
		const LineSegment& candidateSeg)
{
	if (hasBadOutputIntersection(candidateSeg))
		return true;

	if (hasBadInputIntersection(parentLine, sectionIndex, candidateSeg))
		return true;

	return false;
}

/*private*/
bool
TaggedLineStringSimplifier::hasBadOutputIntersection(
		const LineSegment& candidateSeg)
{
	auto_ptr< vector<LineSegment*> > querySegs =
		outputIndex->query(&candidateSeg);

	for (vector<LineSegment*>::iterator
			it = querySegs->begin(), iEnd = querySegs->end();
			it != iEnd;
			++it)
	{
		LineSegment* querySeg = *it;
		assert(querySeg);
		if (hasInteriorIntersection(*querySeg, candidateSeg))
		{
			return true;
		}
	}

	return false;
}

/*private*/
bool
TaggedLineStringSimplifier::hasInteriorIntersection(
			const LineSegment& seg0,
			const LineSegment& seg1) const
{
	li->computeIntersection(seg0.p0, seg0.p1, seg1.p0, seg1.p1);
	return li->isInteriorIntersection();
}

/*private*/
bool
TaggedLineStringSimplifier::hasBadInputIntersection(
		const TaggedLineString* parentLine,
		const vector<unsigned int>& sectionIndex,
		const LineSegment& candidateSeg)
{
	auto_ptr< vector<LineSegment*> > querySegs =
		inputIndex->query(&candidateSeg);

	for (vector<LineSegment*>::iterator
			it = querySegs->begin(), iEnd = querySegs->end();
			it != iEnd;
			++it)
	{
		assert(*it);
		assert(dynamic_cast<TaggedLineSegment*>(*it));
		TaggedLineSegment* querySeg = 
			static_cast<TaggedLineSegment*>(*it);

		if (hasInteriorIntersection(*querySeg, candidateSeg))
		{

			if ( isInLineSection(parentLine,
					sectionIndex, querySeg) )
			{
				continue;
			}

			return true;
		}
	}

	return false;
}

/*static private*/
bool
TaggedLineStringSimplifier::isInLineSection(
		const TaggedLineString* line,
		const vector<unsigned int>& sectionIndex,
		const TaggedLineSegment* seg)
{
	// not in this line
	if (seg->getParent() != line->getParent())
		return false;

	unsigned int segIndex = seg->getIndex();
	if (segIndex >= sectionIndex[0] && segIndex < sectionIndex[1])
		return true;

	return false;
}

/*private*/
void
TaggedLineStringSimplifier::remove(TaggedLineString* line,
		unsigned int start,
		unsigned int end)
{
	assert(end < line->getSegments().size() );
	assert(start < end); // I'm not sure this should always be true

	for (unsigned int i = start; i < end; i++)
	{
		TaggedLineSegment* seg = line->getSegment(i);
		inputIndex->remove(seg);
	}
}

/*private static*/
unsigned int
TaggedLineStringSimplifier::findFarthestPoint(
		const geom::CoordinateSequence* pts,
		unsigned int i, unsigned int j,
		double& maxDistance)
{
	LineSegment seg(pts->getAt(i), pts->getAt(j));
	double maxDist = -1.0;
	unsigned int maxIndex = i;
	for (unsigned int k = i + 1; k < j; k++) {
		const Coordinate& midPt = pts->getAt(k);
		double distance = seg.distance(midPt);
		if (distance > maxDist) {
			maxDist = distance;
			maxIndex = k;
		}
	}
	maxDistance = maxDist;
	return maxIndex;
 
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/12 17:19:57  strk
 * Ported TaggedLineStringSimplifier class, made LineSegment class
 * polymorphic to fix derivation of TaggedLineSegment
 *
 **********************************************************************/
