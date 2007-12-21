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
 **********************************************************************
 *
 * Last port: operation/predicate/RectangleIntersects.java rev 1.3 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/operation/predicate/SegmentIntersectionTester.h>

/// for EnvelopeIntersectsVisitor inheritance
#include <geos/geom/util/ShortCircuitedGeometryVisitor.h>
#include <geos/geom/util/LinearComponentExtracter.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/geom/IntersectionMatrix.h>

#include <geos/algorithm/locate/SimplePointInAreaLocator.h>

//using namespace geos::geom::util;

namespace geos {
namespace operation { // geos.operation
namespace predicate { // geos.operation.predicate

const size_t RectangleIntersects::MAXIMUM_SCAN_SEGMENT_COUNT = 200;


//----------------------------------------------------------------
// EnvelopeIntersectsVisitor
//----------------------------------------------------------------

class EnvelopeIntersectsVisitor: public geom::util::ShortCircuitedGeometryVisitor
{
private:

	const geom::Envelope &rectEnv;
	bool intersectsVar;

protected:

	/**
	 * Reports whether it can be concluded that an intersection occurs,
	 * or whether further testing is required.
	 *
	 * @return <code>true</code> if an intersection must occur
	 * <code>false</code> if no conclusion can be made
	 */
	void visit(const geom::Geometry &element)
	{
		const geom::Envelope &elementEnv = *(element.getEnvelopeInternal());

		// disjoint
		if ( ! rectEnv.intersects(elementEnv) ) {
			return;
		}

		// fully contained - must intersect
		if ( rectEnv.contains(elementEnv) ) {
			intersectsVar = true;
			return;
		}

		/*
		 * Since the envelopes intersect and the test element is
		 * connected, if the test envelope is completely bisected by
		 * an edge of the rectangle the element and the rectangle
		 * must touch (This is basically an application of the
		 * Jordan Curve Theorem).  The alternative situation
		 * is that the test envelope is "on a corner" of the
		 * rectangle envelope, i.e. is not completely bisected.
		 * In this case it is not possible to make a conclusion
		 * about the presence of an intersection.
		 */
		if (elementEnv.getMinX() >= rectEnv.getMinX()
			&& elementEnv.getMaxX() <= rectEnv.getMaxX())
		{
			intersectsVar=true;
			return;
		}
		if (elementEnv.getMinY() >= rectEnv.getMinY()
			&& elementEnv.getMaxY() <= rectEnv.getMaxY())
		{
			intersectsVar = true;
			return;
		}
	}

	bool isDone() { return intersectsVar==true; }

public:

	EnvelopeIntersectsVisitor(const geom::Envelope &env)
		:
		rectEnv(env),
		intersectsVar(false)
		{}

	bool intersects() { return intersectsVar; }

};

//----------------------------------------------------------------
// ContainsPointVisitor
//----------------------------------------------------------------

/**
 * Tests whether it can be concluded
 * that a geometry contains a corner point of a rectangle.
 */
class ContainsPointVisitor: public geom::util::ShortCircuitedGeometryVisitor
{
private:

	const geom::Envelope &rectEnv;
	bool containsPointVar;
	const geom::CoordinateSequence &rectSeq;

protected:

	void visit(const geom::Geometry &geom)
	{
		using geos::algorithm::locate::SimplePointInAreaLocator;

		const geom::Polygon *poly;

		if ( !(poly=dynamic_cast<const geom::Polygon *>(&geom)) ) {
			return;
		}

		const geom::Envelope &elementEnv = *(geom.getEnvelopeInternal());

		if ( !rectEnv.intersects(elementEnv) ) {
			return;
		}

		// test each corner of rectangle for inclusion
		for (int i=0; i<4; i++)
		{

			const geom::Coordinate &rectPt=rectSeq.getAt(i);

			if ( !elementEnv.contains(rectPt) ) {
				continue;
			}

			// check rect point in poly (rect is known not to
			// touch polygon at this point)
			if ( SimplePointInAreaLocator::containsPointInPolygon(rectPt, poly) )
			{
				containsPointVar=true;
				return;
			}
		}
	}

	bool isDone() { return containsPointVar; }

public:

	ContainsPointVisitor(const geom::Polygon &rect)
		:
		rectEnv(*(rect.getEnvelopeInternal())),
		containsPointVar(false),
		rectSeq(*(rect.getExteriorRing()->getCoordinatesRO()))
		{}

	bool containsPoint() { return containsPointVar; }

};

//----------------------------------------------------------------
// LineIntersectsVisitor
//----------------------------------------------------------------

class LineIntersectsVisitor: public geom::util::ShortCircuitedGeometryVisitor
{
private:

	const geom::Polygon& rectangle;
	const geom::Envelope& rectEnv;
	bool intersectsVar;
	const geom::CoordinateSequence &rectSeq;

	void computeSegmentIntersection(const geom::Geometry &geom)
	{
		using geos::geom::util::LinearComponentExtracter;

		// check segment intersection
		// get all lines from geom (e.g. if it's a multi-ring polygon)
		geom::LineString::ConstVect lines;
		LinearComponentExtracter::getLines(geom, lines);
		SegmentIntersectionTester si;
		if ( si.hasIntersectionWithLineStrings(rectSeq, lines) )
		{
			intersectsVar = true;
			return;
		}
	}

protected:

	void visit(const geom::Geometry &geom)
	{
		const geom::Envelope &elementEnv = *(geom.getEnvelopeInternal());
		if (! rectEnv.intersects(elementEnv) ) {
			return;
		}

		// check if general relate algorithm should be used,
		// since it's faster for large inputs
		if (geom.getNumPoints() > RectangleIntersects::MAXIMUM_SCAN_SEGMENT_COUNT)
		{
			intersectsVar = rectangle.relate(geom)->isIntersects();
			return;
		}

		// if small enough, test for segment intersection directly
		computeSegmentIntersection(geom);
	}

	bool isDone() { return intersectsVar; }

public:

	LineIntersectsVisitor(const geom::Polygon &rect)
		:
		rectangle(rect),
		rectEnv(*(rect.getEnvelopeInternal())),
		intersectsVar(false),
		rectSeq(*(rect.getExteriorRing()->getCoordinatesRO()))
		{}

	/**
	 * Reports whether any segment intersection exists.
	 *
	 * @return <code>true</code> if a segment intersection exists
	 * <code>false</code> if no segment intersection exists
	 */
	bool intersects() { return intersectsVar; }

};

//----------------------------------------------------------------
// RectangleIntersects
//----------------------------------------------------------------

bool
RectangleIntersects::intersects(const geom::Geometry& geom)
{
	if (!rectEnv.intersects(geom.getEnvelopeInternal()))
		return false;

	// test envelope relationships
	EnvelopeIntersectsVisitor visitor(rectEnv);
	visitor.applyTo(geom);
	if (visitor.intersects()) return true;

	// test if any rectangle corner is contained in the target
	ContainsPointVisitor ecpVisitor(rectangle);
	ecpVisitor.applyTo(geom);
	if (ecpVisitor.containsPoint())
		return true;

	// test if any lines intersect
	LineIntersectsVisitor liVisitor(rectangle);
	liVisitor.applyTo(geom);
	if (liVisitor.intersects())
		return true;

	return false;
}

} // namespace geos.operation.predicate
} // namespace geos.operation
} // namespace geos



