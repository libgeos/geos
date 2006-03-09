/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_LINESEGMENT_H
#define GEOS_GEOM_LINESEGMENT_H

#include <iostream> // for ostream

#include <geos/geom/Coordinate.h>

#include <geos/inline.h>

namespace geos {
namespace geom { // geos::geom

/**
 * Represents a line segment defined by two Coordinate.
 * Provides methods to compute various geometric properties
 * and relationships of line segments.
 * 
 * This class is designed to be easily mutable (to the extent of
 * having its contained points public).
 * This supports a common pattern of reusing a single LineSegment
 * object as a way of computing segment properties on the
 * segments defined by arrays or lists of {@link Coordinate}s.
 */
class LineSegment {
public:

	friend std::ostream& operator<< (std::ostream& o, const LineSegment& l);

	Coordinate p0; /// Segment start

	Coordinate p1; /// Segemnt end

	LineSegment();

	LineSegment(const LineSegment &ls);

	/// Constructs a LineSegment with the given start and end Coordinates.
	LineSegment(const Coordinate& c0, const Coordinate& c1);

	~LineSegment();

	void setCoordinates(const Coordinate& c0, const Coordinate& c1);

	const Coordinate& getCoordinate(unsigned int i) const;

	void setCoordinates(const LineSegment& ls);

	/// Computes the length of the line segment.
	double getLength() const;

	/// Tests whether the segment is horizontal.
	//
	/// @return <code>true</code> if the segment is horizontal
	///
	bool isHorizontal() const;

	/// Tests whether the segment is vertical.
	//
	/// @return <code>true</code> if the segment is vertical
	///
	bool isVertical() const;

	/**
	 * Determines the orientation of a LineSegment relative to this segment.
	 * The concept of orientation is specified as follows:
	 * Given two line segments A and L,
	 * <ul
	 * <li>A is to the left of a segment L if A lies wholly in the
	 * closed half-plane lying to the left of L
	 * <li>A is to the right of a segment L if A lies wholly in the
	 * closed half-plane lying to the right of L
	 * <li>otherwise, A has indeterminate orientation relative to L.
	 *     This happens if A is collinear with L or if A crosses
	 *     the line determined by L.
	 * </ul>
	 *
	 * @param seg the LineSegment to compare
	 *
	 * @return 1 if seg is to the left of this segment
	 * @return -1 if seg is to the right of this segment
	 * @return 0 if seg has indeterminate orientation relative
	 *	     to this segment
	 */
	int orientationIndex(const LineSegment& seg) const;

	int orientationIndex(const LineSegment* seg) const;

	void reverse();

	void normalize();

	double angle() const;

	double distance(const LineSegment ls) const;

	/**
	 * Computes the distance between this line segment and a point.
	 */
	double distance(const Coordinate& p) const;

	/** \brief
	 * Computes the perpendicular distance between the (infinite)
	 * line defined by this line segment and a point.
	 */
	double distancePerpendicular(const Coordinate& p) const;

	double projectionFactor(const Coordinate& p) const;

	/** \brief
	 * Compute the projection of a point onto the line determined
	 * by this line segment.
	 * 
	 * Note that the projected point
	 * may lie outside the line segment.  If this is the case,
	 * the projection factor will lie outside the range [0.0, 1.0].
	 */
	void project(const Coordinate& p, Coordinate& ret) const;

	/** \brief
	 * Project a line segment onto this line segment and return the resulting
	 * line segment. 
	 *
	 * The returned line segment will be a subset of
	 * the target line line segment.  This subset may be null, if
	 * the segments are oriented in such a way that there is no projection.
	 * 
	 * Note that the returned line may have zero length (i.e. the same endpoints).
	 * This can happen for instance if the lines are perpendicular to one another.
	 *
	 * @param seg the line segment to project
	 * @param ret the projected line segment
	 * @return true if there is an overlap, false otherwise
	 */
	bool project(const LineSegment& seg, LineSegment& ret) const;

	/// Computes the closest point on this line segment to another point.
	//
	/// @param p the point to find the closest point to
	/// @param ret the Coordinate to which the closest point on the line segment
	///            to the point p will be written
	///
	void closestPoint(const Coordinate& p, Coordinate& ret) const;

	/** \brief
	 * Compares this object with the specified object for order.
	 *
	 * Uses the standard lexicographic ordering for the points in the LineSegment.
	 *
	 * @param  o  the LineSegment with which this LineSegment
	 *            is being compared
	 * @return a negative integer, zero, or a positive integer as this
	 *         LineSegment is less than, equal to, or greater than the
	 *         specified LineSegment
	 */
	int compareTo(const LineSegment& other) const;

	/** \brief
	 *  Returns <code>true</code> if <code>other</code> is
	 *  topologically equal to this LineSegment (e.g. irrespective
	 *  of orientation).
	 *
	 * @param  other  a <code>LineSegment</code> with which to do the comparison.
	 * @return true if other is a LineSegment
	 *      with the same values for the x and y ordinates.
	 */
	bool equalsTopo(const LineSegment& other) const;

	/**
	 * Computes the closest points on two line segments.
	 * @param p the point to find the closest point to
	 * @return a pair of Coordinates which are the closest points on
	 * the line segments.
	 * The returned CoordinateList must be deleted by caller
	 */
	CoordinateSequence* closestPoints(const LineSegment& line);

	CoordinateSequence* closestPoints(const LineSegment* line);

	/**
	 * Computes an intersection point between two segments,
	 * if there is one.
	 * There may be 0, 1 or many intersection points between two segments.
	 * If there are 0, null is returned. If there is 1 or more, a single
	 * one is returned (chosen at the discretion of the algorithm). 
	 * If more information is required about the details of the
	 * intersection, the LineIntersector class should be used.
	 *
	 * @param line
	 * @param coord the Coordinate to write the result into
	 * @return true if an intersection was found, false otherwise
	 */
	bool intersection(const LineSegment& line, Coordinate& coord) const;

};

std::ostream& operator<< (std::ostream& o, const LineSegment& l);

/// Checks if two LineSegment are equal (2D only check)
bool operator==(const LineSegment& a, const LineSegment& b);


} // namespace geos::geom
} // namespace geos

#ifdef USE_INLINE
# include "geos/geom/LineSegment.inl"
#endif

#endif // ndef GEOS_GEOM_LINESEGMENT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
