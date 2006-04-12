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
 **********************************************************************
 *
 * NOTES: This class can be optimized to work with vector<Coordinate*>
 *        rather then with CoordinateSequence
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_TAGGEDLINESTRINGSIMPLIFIER_H_
#define _GEOS_SIMPLIFY_TAGGEDLINESTRINGSIMPLIFIER_H_ 

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
	namespace algorithm {
		class LineIntersector;
	}
	namespace geom {
		//class Coordinate;
		class CoordinateSequence;
		class LineSegment;
		//class Geometry;
		//class LineString;
		//class LinearRing;
	}
	namespace simplify {
		class TaggedLineSegment;
		class TaggedLineString;
		class LineSegmentIndex;
	}
}

namespace geos {
namespace simplify { // geos::simplify


/** \brief
 * Simplifies a TaggedLineString, preserving topology
 * (in the sense that no new intersections are introduced).
 * Uses the recursive Douglas-Peucker algorithm.
 *
 */
class TaggedLineStringSimplifier {

public:

	TaggedLineStringSimplifier(LineSegmentIndex* inputIndex,
		LineSegmentIndex* outputIndex);

	/** \brief
	 * Sets the distance tolerance for the simplification.
	 *
	 * All vertices in the simplified geometry will be within this
	 * distance of the original geometry.
	 *
	 * @param d the approximation tolerance to use
	 */
	void setDistanceTolerance(double d);

	void simplify(TaggedLineString* line);


private:

	LineSegmentIndex* inputIndex;

	LineSegmentIndex* outputIndex;

	std::auto_ptr<algorithm::LineIntersector> li;

	/// non-const as segments are possibly added to it
	TaggedLineString* line;

	const geom::CoordinateSequence* linePts;

	double distanceTolerance;

	void simplifySection(unsigned int i, unsigned int j,
			unsigned int depth);

	static unsigned int findFarthestPoint(
			const geom::CoordinateSequence* pts,
			unsigned int i, unsigned int j,
			double& maxDistance);

	bool hasBadIntersection(const TaggedLineString* parentLine,
                       const std::vector<unsigned int>& sectionIndex,
                       const geom::LineSegment& candidateSeg);

	bool hasBadInputIntersection(const TaggedLineString* parentLine,
                       const std::vector<unsigned int>& sectionIndex,
                       const geom::LineSegment& candidateSeg);

	bool hasBadOutputIntersection(const geom::LineSegment& candidateSeg);

	bool hasInteriorIntersection(const geom::LineSegment& seg0,
			const geom::LineSegment& seg1) const;

	std::auto_ptr<TaggedLineSegment> flatten(
			unsigned int start, unsigned int end);

	/** \brief
	 * Tests whether a segment is in a section of a TaggedLineString
	 *
	 * @param line
	 * @param sectionIndex
	 * @param seg
	 * @return
	 */
	static bool isInLineSection(
		const TaggedLineString* parentLine,
		const std::vector<unsigned int>& sectionIndex,
		const TaggedLineSegment* seg);

	/** \brief
	 * Remove the segs in the section of the line
	 *
	 * @param line
	 * @param pts
	 * @param sectionStartIndex
	 * @param sectionEndIndex
	 */
	void remove(TaggedLineString* line, unsigned int start,
			unsigned int end);
 
};

inline void
TaggedLineStringSimplifier::setDistanceTolerance(double d)
{
	distanceTolerance = d;
}

} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_TAGGEDLINESTRINGSIMPLIFIER_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/12 17:19:57  strk
 * Ported TaggedLineStringSimplifier class, made LineSegment class
 * polymorphic to fix derivation of TaggedLineSegment
 *
 **********************************************************************/
