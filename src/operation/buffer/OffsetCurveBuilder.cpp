/**********************************************************************
 * $Id$
 *
 * GEOS-Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2005 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev. 1.30 (JTS-1.10)
 *
 **********************************************************************/

#include <cassert>
#include <cmath>
#include <vector>

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/Angle.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>
#include <geos/operation/buffer/OffsetCurveVertexList.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/geomgraph/Position.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/algorithm/HCoordinate.h>
#include <geos/util.h>

#include "BufferInputLineSimplifier.h"

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

// Define this to skip the input simplification step (for testing)
//#define SKIP_INPUT_SIMPLIFICATION

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/*private data*/
const double OffsetCurveBuilder::CURVE_VERTEX_SNAP_DISTANCE_FACTOR = 1.0E-6;
const double OffsetCurveBuilder::PI = 3.14159265358979;
const double OffsetCurveBuilder::MAX_CLOSING_SEG_LEN = 3.0;
const double OffsetCurveBuilder::OFFSET_SEGMENT_SEPARATION_FACTOR = 1.0E-3;
const double OffsetCurveBuilder::INSIDE_TURN_VERTEX_SNAP_DISTANCE_FACTOR = 1.0E-3;
const double OffsetCurveBuilder::SIMPLIFY_FACTOR = 100.0;

/*public*/
OffsetCurveBuilder::OffsetCurveBuilder(const PrecisionModel *newPrecisionModel,
		const BufferParameters& nBufParams)
		:
		li(),
		maxCurveSegmentError(0.0),
		vertexList(),
		distance(0.0),
		precisionModel(newPrecisionModel),
		bufParams(nBufParams),
		closingSegFactor(1),
		s0(),
		s1(),
		s2(),
		seg0(),
		seg1(),
		offset0(),
		offset1(),
		side(0),
		endCapIndex(0)
{
	// compute intersections in full precision, to provide accuracy
	// the points are rounded as they are inserted into the curve line
	filletAngleQuantum = PI / 2.0 / bufParams.getQuadrantSegments();

	/**
	 * Non-round joins cause issues with short closing segments,
	 * so don't use them.  In any case, non-round joins
	 * only really make sense for relatively small buffer distances.
	 */
	if (bufParams.getQuadrantSegments() >= 8
	     && bufParams.getJoinStyle() == BufferParameters::JOIN_ROUND)
	{
		closingSegFactor = MAX_CLOSING_SEG_FRACTION;
	}
}

/*public*/
OffsetCurveBuilder::~OffsetCurveBuilder()
{
}


/*public*/
void
OffsetCurveBuilder::getLineCurve(const CoordinateSequence *inputPts,
		double distance, vector<CoordinateSequence*>& lineList)
{
	// a zero or negative width buffer of a line/point is empty
	if (distance<= 0.0) return;

	init(distance);

	if (inputPts->getSize() <= 1) {
		switch (bufParams.getEndCapStyle()) {
			case BufferParameters::CAP_ROUND:
				addCircle(inputPts->getAt(0), distance);
				break;
			case BufferParameters::CAP_SQUARE:
				addSquare(inputPts->getAt(0), distance);
				break;
			default:
				// default is for buffer to be empty
				// (e.g. for a butt line cap);
				break;
		}
	} else {
		computeLineBufferCurve(*inputPts);
	}

	// NOTE: we take ownership of lineCoord here ...
	CoordinateSequence *lineCoord=vertexList.getCoordinates();

	// ... and we give it away here
	lineList.push_back(lineCoord);
}

/*public*/
void
OffsetCurveBuilder::getSingleSidedLineCurve(const CoordinateSequence* inputPts, 
   double distance, vector<CoordinateSequence*>& lineList, bool leftSide,
   bool rightSide)
{
   // A zero or negative width buffer of a line/point is empty.
   if ( distance <= 0.0 ) return ;

   init( distance ) ;

   if ( inputPts->getSize() < 2 )
   {
      // No cap, so just return.
      return ;
   }
   else
   {
      computeLineBufferCurve( *inputPts ) ;
   }

   // NOTE: we take ownership of lineCoord here ...
   std::auto_ptr<CoordinateSequence> lineCoord (vertexList.getCoordinates());

   // [strk] Oct 1, 2009
   // Left side:  index [n-1] to [endCapIndex]
   // Right side: index [endCapIndex+1] to [n-2]
   // Where n is the last index (size-1).
   int n = lineCoord->size() - 1 ;

   // Add the left side curve to the line list.
   if ( leftSide )
   {
      CoordinateArraySequence* coordSeq = new CoordinateArraySequence() ;
      //coordSeq->add( ( *lineCoord )[n-2] ) ;
      coordSeq->add( ( *lineCoord )[n-1] ) ;
      for ( int i = 0 ; i <= endCapIndex ; ++i )
      {
         coordSeq->add( ( *lineCoord )[i] ) ;
      }
      lineList.push_back( coordSeq ) ;
   }

   // Add the right side curve to the line list.
   if ( rightSide )
   {
      CoordinateArraySequence* coordSeq = new CoordinateArraySequence() ;
      for ( int i = endCapIndex+1 ; i <= n-2 ; ++i )
      {
         coordSeq->add( ( *lineCoord )[i] ) ;
      }

      lineList.push_back( coordSeq ) ;
   }
}

/*public*/
void
OffsetCurveBuilder::getRingCurve(const CoordinateSequence *inputPts,
		int side, double distance,
		vector<CoordinateSequence*>& lineList)
{
	init(distance);
	if (inputPts->getSize() <= 2)
	{
		getLineCurve(inputPts, distance, lineList);
		return;
	}

	// optimize creating ring for zero distance
	if (distance == 0.0) {
		vertexList.reset(); // is this needed ?
		lineList.push_back(inputPts->clone());
		return;
	}

	computeRingBufferCurve(*inputPts, side);

	// this will be vertexList
	// NOTE: getCoordinates() take ownership of the CoordinateSequence
	lineList.push_back(vertexList.getCoordinates());
}

/*private*/
void
OffsetCurveBuilder::init(double newDistance)
{
	distance = newDistance;
	maxCurveSegmentError = distance * (1 - cos(filletAngleQuantum/2.0));

	// Point list needs to be reset
	// but if a previous point list exists
	// we'd better back it up for final deletion
	vertexList.reset();
	vertexList.setPrecisionModel(precisionModel);

	/**
	 * Choose the min vertex separation as a small fraction of
	 * the offset distance.
	 */
	vertexList.setMinimumVertexDistance(
		distance * CURVE_VERTEX_SNAP_DISTANCE_FACTOR);
}

/* private */
double
OffsetCurveBuilder::simplifyTolerance(double bufDistance)
{
	return bufDistance / SIMPLIFY_FACTOR;
}

/*private*/
void
OffsetCurveBuilder::computeLineBufferCurve(const CoordinateSequence& inputPts)
{
	double distTol = simplifyTolerance(distance);

	//--------- compute points for left side of line
#ifndef SKIP_INPUT_SIMPLIFICATION
	// Simplify the appropriate side of the line before generating
	std::auto_ptr<CoordinateSequence> simp1_ = 
		BufferInputLineSimplifier::simplify(inputPts, distTol);
	const CoordinateSequence& simp1 = *simp1_;
#else
	// MD - used for testing only (to eliminate simplification)
	const CoordinateSequence& simp1 = inputPts;
#endif


	int n1 = simp1.size() - 1;
	initSideSegments(simp1[0], simp1[1], Position::LEFT);
	for (int i = 2; i <= n1; ++i) {
		addNextSegment(simp1[i], true);
	}
	addLastSegment();
	// add line cap for end of line
	addLineEndCap(simp1[n1-1], simp1[n1]);

	// Record the index of the end of line cap.
	endCapIndex = vertexList.size() - 2 ;


	//---------- compute points for right side of line
#ifndef SKIP_INPUT_SIMPLIFICATION
	// Simplify the appropriate side of the line before generating
	std::auto_ptr<CoordinateSequence> simp2_ = 
		BufferInputLineSimplifier::simplify(inputPts, -distTol);
	const CoordinateSequence& simp2 = *simp2_;
#else
	// MD - used for testing only (to eliminate simplification)
	const CoordinateSequence& simp2 = inputPts;
#endif

	int n2 = simp2.size() - 1;
	initSideSegments(simp2[n2], simp2[n2-1], Position::LEFT);
	for (int i = n2-2; i >= 0; --i) {
		addNextSegment(simp2[i], true);
	}
	addLastSegment();
	// add line cap for start of line
	addLineEndCap(simp2[1], simp2[0]);

	vertexList.closeRing();
}

/*private*/
void
OffsetCurveBuilder::computeRingBufferCurve(const CoordinateSequence& inputPts,
	int side)
{

#ifndef SKIP_INPUT_SIMPLIFICATION
	double distTol = simplifyTolerance(distance);
	// ensure that correct side is simplified
	if (side == Position::RIGHT)
		distTol = -distTol;      
	std::auto_ptr<CoordinateSequence> simp_ = 
		BufferInputLineSimplifier::simplify(inputPts, distTol);
	const CoordinateSequence& simp = *simp_;
#else
	const CoordinateSequence& simp = inputPts;
#endif

	int n = simp.size()-1;
	initSideSegments(simp[n-1], simp[0], side);
	for (int i = 1; i <= n; i++) {
		bool addStartPoint = i != 1;
		addNextSegment(simp[i], addStartPoint);
	}
	vertexList.closeRing();
}

/*private*/
void
OffsetCurveBuilder::initSideSegments(const Coordinate &nS1,
		const Coordinate &nS2, int nSide)
{
	s1=nS1;
	s2=nS2;
	side=nSide;
	seg1.setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);
}

/*private*/
void
OffsetCurveBuilder::addNextSegment(const Coordinate &p, bool addStartPoint)
{
	// s0-s1-s2 are the coordinates of the previous segment
	// and the current one
	s0=s1;
	s1=s2;
	s2=p;
	seg0.setCoordinates(s0, s1);
	computeOffsetSegment(seg0, side, distance, offset0);
	seg1.setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);

	// do nothing if points are equal
	if (s1==s2) return;

	int orientation=CGAlgorithms::computeOrientation(s0, s1, s2);
	bool outsideTurn =
		(orientation==CGAlgorithms::CLOCKWISE
		 && side==Position::LEFT)
		||
		(orientation==CGAlgorithms::COUNTERCLOCKWISE 
		 && side==Position::RIGHT);

	if (orientation==0)
	{
		// lines are collinear
		addCollinear(addStartPoint);
	}
	else if (outsideTurn)
	{
		addOutsideTurn(orientation, addStartPoint);
	}
	else
	{
		// inside turn
		addInsideTurn(orientation, addStartPoint);
	}
}

/*private*/
void
OffsetCurveBuilder::addLastSegment()
{
	vertexList.addPt(offset1.p1);
}

/*private*/
void
OffsetCurveBuilder::computeOffsetSegment(const LineSegment& seg, int side,
	double distance, LineSegment& offset)
{
	int sideSign = side == Position::LEFT ? 1 : -1;
	double dx = seg.p1.x - seg.p0.x;
	double dy = seg.p1.y - seg.p0.y;
	double len = sqrt(dx * dx + dy * dy);
	// u is the vector that is the length of the offset,
	// in the direction of the segment
	double ux = sideSign * distance * dx / len;
	double uy = sideSign * distance * dy / len;
	offset.p0.x = seg.p0.x - uy;
	offset.p0.y = seg.p0.y + ux;
	offset.p1.x = seg.p1.x - uy;
	offset.p1.y = seg.p1.y + ux;
}

/*private*/
void
OffsetCurveBuilder::addLineEndCap(const Coordinate &p0, const Coordinate &p1)
{
	LineSegment seg(p0, p1);

	LineSegment offsetL;
	computeOffsetSegment(seg, Position::LEFT, distance, offsetL);
	LineSegment offsetR;
	computeOffsetSegment(seg, Position::RIGHT, distance, offsetR);

	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	double angle=atan2(dy, dx);

	switch (bufParams.getEndCapStyle()) {
		case BufferParameters::CAP_ROUND:
			// add offset seg points with a fillet between them
			vertexList.addPt(offsetL.p1);
			addFillet(p1, angle+PI/2.0, angle-PI/2.0,
			          CGAlgorithms::CLOCKWISE, distance);
			vertexList.addPt(offsetR.p1);
			break;
		case BufferParameters::CAP_FLAT:
			// only offset segment points are added
			vertexList.addPt(offsetL.p1);
			vertexList.addPt(offsetR.p1);
			break;
		case BufferParameters::CAP_SQUARE:
			// add a square defined by extensions of the offset
			// segment endpoints
			Coordinate squareCapSideOffset;
			squareCapSideOffset.x=fabs(distance)*cos(angle);
			squareCapSideOffset.y=fabs(distance)*sin(angle);

			Coordinate squareCapLOffset(
				offsetL.p1.x+squareCapSideOffset.x,
				offsetL.p1.y+squareCapSideOffset.y);
			Coordinate squareCapROffset(
				offsetR.p1.x+squareCapSideOffset.x,
				offsetR.p1.y+squareCapSideOffset.y);
			vertexList.addPt(squareCapLOffset);
			vertexList.addPt(squareCapROffset);
			break;
	}
}

/*private*/
void
OffsetCurveBuilder::addFillet(const Coordinate &p, const Coordinate &p0,
	const Coordinate &p1, int direction, double radius)
{
	double dx0 = p0.x - p.x;
	double dy0 = p0.y - p.y;
	double startAngle = atan2(dy0, dx0);
	double dx1 = p1.x - p.x;
	double dy1 = p1.y - p.y;
	double endAngle = atan2(dy1, dx1);

	if (direction == CGAlgorithms::CLOCKWISE) {
		if (startAngle <= endAngle) startAngle += 2.0 * PI;
	}
	else {    // direction==COUNTERCLOCKWISE
		if (startAngle >= endAngle) startAngle -= 2.0 * PI;
	}

	vertexList.addPt(p0);
	addFillet(p, startAngle, endAngle, direction, radius);
	vertexList.addPt(p1);
}

/*private*/
void
OffsetCurveBuilder::addFillet(const Coordinate &p, double startAngle,
	double endAngle, int direction, double radius)
{
	int directionFactor = direction == CGAlgorithms::CLOCKWISE ? -1 : 1;

	double totalAngle = fabs(startAngle - endAngle);
	int nSegs = (int) (totalAngle / filletAngleQuantum + 0.5);

	// no segments because angle is less than increment-nothing to do!
	if (nSegs<1) return;

	double initAngle, currAngleInc;

	// choose angle increment so that each segment has equal length
	initAngle = 0.0;
	currAngleInc = totalAngle / nSegs;

	double currAngle = initAngle;
	Coordinate pt;
	while (currAngle < totalAngle) {
		double angle = startAngle + directionFactor * currAngle;
		pt.x = p.x + radius * cos(angle);
		pt.y = p.y + radius * sin(angle);
		vertexList.addPt(pt);
		currAngle += currAngleInc;
	}
}


/*private*/
void
OffsetCurveBuilder::addCircle(const Coordinate &p, double distance)
{
	// add start point
	Coordinate pt(p.x + distance, p.y);
	vertexList.addPt(pt);
	addFillet(p, 0.0, 2.0*PI, -1, distance);
}

/*private*/
void
OffsetCurveBuilder::addSquare(const Coordinate &p, double distance)
{
	// add start point
	vertexList.addPt(Coordinate(p.x+distance, p.y+distance));
	vertexList.addPt(Coordinate(p.x+distance, p.y-distance));
	vertexList.addPt(Coordinate(p.x-distance, p.y-distance));
	vertexList.addPt(Coordinate(p.x-distance, p.y+distance));
	vertexList.addPt(Coordinate(p.x+distance, p.y+distance));
}

/* private */
void
OffsetCurveBuilder::addCollinear(bool addStartPoint)
{
	/**
	 * This test could probably be done more efficiently,
	 * but the situation of exact collinearity should be fairly rare.
	 */

	li.computeIntersection(s0,s1,s1,s2);
	int numInt=li.getIntersectionNum();

	/**
	 * if numInt is<2, the lines are parallel and in the same direction.
	 * In this case the point can be ignored, since the offset lines
	 * will also be parallel.
	 */
	if (numInt>= 2)
	{
		/**
		 * Segments are collinear but reversing. 
		 * Add an "end-cap" fillet
		 * all the way around to other direction
		 *
		 * This case should ONLY happen for LineStrings,
		 * so the orientation is always CW (Polygons can never
		 * have two consecutive segments which are parallel but
		 * reversed, because that would be a self intersection).
		 */
		if (  bufParams.getJoinStyle() == BufferParameters::JOIN_BEVEL
		   || bufParams.getJoinStyle() == BufferParameters::JOIN_MITRE)
		{
			if (addStartPoint) vertexList.addPt(offset0.p1);
			vertexList.addPt(offset1.p0);
		}
		else
		{
			addFillet(s1, offset0.p1, offset1.p0,
				  CGAlgorithms::CLOCKWISE, distance);
		}
	}
}

/* private */
void
OffsetCurveBuilder::addOutsideTurn(int orientation, bool addStartPoint)
{
	/**
	 * Heuristic: If offset endpoints are very close together,
	 * just use one of them as the corner vertex.
	 * This avoids problems with computing mitre corners in the case
	 * where the two segments are almost parallel
	 * (which is hard to compute a robust intersection for).
	 */

	if (offset0.p1.distance(offset1.p0) <
		distance*OFFSET_SEGMENT_SEPARATION_FACTOR)
	{
		vertexList.addPt(offset0.p1);
		return;
	}

	if (bufParams.getJoinStyle() == BufferParameters::JOIN_MITRE)
	{
		addMitreJoin(s1, offset0, offset1, distance);
	}
	else if (bufParams.getJoinStyle() == BufferParameters::JOIN_BEVEL)
	{
		addBevelJoin(offset0, offset1);
	}
	else
	{
		// add a circular fillet connecting the endpoints 
		// of the offset segments
		if (addStartPoint) vertexList.addPt(offset0.p1);

		// TESTING - comment out to produce beveled joins
		addFillet(s1, offset0.p1, offset1.p0, orientation, distance);
		vertexList.addPt(offset1.p0);
	}
}

/* private */
void
OffsetCurveBuilder::addInsideTurn(int orientation, bool addStartPoint)
{
    ::geos::ignore_unused_variable_warning(orientation);
    ::geos::ignore_unused_variable_warning(addStartPoint);

	// add intersection point of offset segments (if any)
	li.computeIntersection(offset0.p0, offset0.p1, offset1.p0, offset1.p1);
	if (li.hasIntersection())
	{
		vertexList.addPt(li.getIntersection(0));
		return;
	}

	// If no intersection is detected, it means the angle is so small
	// and/or the offset so large that the offsets segments don't
	// intersect. In this case we must add a "closing segment" to make
	// sure the buffer curve is continuous,
	// fairly smooth (e.g. no sharp reversals in direction)
	// and tracks the buffer correctly around the corner.
	// The curve connects the endpoints of the segment offsets to points
	// which lie toward the centre point of the corner.
	// The joining curve will not appear in the final buffer outline,
	// since it is completely internal to the buffer polygon.
	//
	// In complex buffer cases the closing segment may cut across many
	// other segments in the generated offset curve. 
	// In order to improve the performance of the noding, the closing
	// segment should be kept as short as possible.
	// (But not too short, since that would defeat it's purpose).
	// This is the purpose of the closingSegFactor heuristic value.

       /**
        * The intersection test above is vulnerable to robustness errors;
	* i.e. it may be that the offsets should intersect very close to
	* their endpoints, but aren't reported as such due to rounding.
	* To handle this situation appropriately, we use the following test:
	* If the offset points are very close, don't add closing segments
	* but simply use one of the offset points
        */

	if (offset0.p1.distance(offset1.p0) <
		distance * INSIDE_TURN_VERTEX_SNAP_DISTANCE_FACTOR)
	{
		vertexList.addPt(offset0.p1);
	}
	else
	{
		// add endpoint of this segment offset
		vertexList.addPt(offset0.p1);

		// Add "closing segment" of required length.
		if ( closingSegFactor > 0 )
		{
			Coordinate mid0(
 (closingSegFactor*offset0.p1.x + s1.x)/(closingSegFactor + 1),
 (closingSegFactor*offset0.p1.y + s1.y)/(closingSegFactor + 1)
			);
			vertexList.addPt(mid0);

			Coordinate mid1(
 (closingSegFactor*offset1.p0.x + s1.x)/(closingSegFactor + 1),
 (closingSegFactor*offset1.p0.y + s1.y)/(closingSegFactor + 1)
			);
			vertexList.addPt(mid1);
		}
		else
		{
			// This branch is not expected to be used
			// except for testing purposes.
			// It is equivalent to the JTS 1.9 logic for
			// closing segments (which results in very poor
			// performance for large buffer distances)
			vertexList.addPt(s1);
		}

		// add start point of next segment offset
		vertexList.addPt(offset1.p0);
	}
}

/* private */
void
OffsetCurveBuilder::addMitreJoin(const geom::Coordinate& p,
	                  const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1,
	                  double distance)
{
	bool isMitreWithinLimit = true;
	Coordinate intPt;

        /**
         * This computation is unstable if the offset segments
	 * are nearly collinear.
         * Howver, this situation should have been eliminated earlier
	 * by the check for whether the offset segment endpoints are
	 * almost coincident
         */
    try
    {
        HCoordinate::intersection(offset0.p0, offset0.p1,
            offset1.p0, offset1.p1,
            intPt);

        double mitreRatio = distance <= 0.0 ? 1.0
            : intPt.distance(p) / fabs(distance);

        if (mitreRatio > bufParams.getMitreLimit())
            isMitreWithinLimit = false;
    }
    catch (const NotRepresentableException& e)
    {
        ::geos::ignore_unused_variable_warning(e);

        intPt = Coordinate(0,0);
        isMitreWithinLimit = false;
    }

    if (isMitreWithinLimit)
    {
        vertexList.addPt(intPt);
    }
    else
    {
        addLimitedMitreJoin(offset0, offset1, distance,
            bufParams.getMitreLimit());
        //addBevelJoin(offset0, offset1);
    }
}

/* private */
void
OffsetCurveBuilder::addLimitedMitreJoin(
	                  const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1,
	                  double distance, double mitreLimit)
{
    ::geos::ignore_unused_variable_warning(offset0);
    ::geos::ignore_unused_variable_warning(offset1);

	const Coordinate& basePt = seg0.p1;

	double ang0 = Angle::angle(basePt, seg0.p0);
	//double ang1 = Angle::angle(basePt, seg1.p1); // unused in JTS, bug ?

	// oriented angle between segments
	double angDiff = Angle::angleBetweenOriented(seg0.p0, basePt, seg1.p1);
	// half of the interior angle
	double angDiffHalf = angDiff / 2;

	// angle for bisector of the interior angle between the segments
	double midAng = Angle::normalize(ang0 + angDiffHalf);
	// rotating this by PI gives the bisector of the reflex angle
	double mitreMidAng = Angle::normalize(midAng + PI);

	// the miterLimit determines the distance to the mitre bevel
	double mitreDist = mitreLimit * distance;
	// the bevel delta is the difference between the buffer distance
	// and half of the length of the bevel segment
	double bevelDelta = mitreDist * fabs(sin(angDiffHalf));
	double bevelHalfLen = distance - bevelDelta;

	// compute the midpoint of the bevel segment
	double bevelMidX = basePt.x + mitreDist * cos(mitreMidAng);
	double bevelMidY = basePt.y + mitreDist * sin(mitreMidAng);
	Coordinate bevelMidPt(bevelMidX, bevelMidY);

	// compute the mitre midline segment from the corner point to
	// the bevel segment midpoint
	LineSegment mitreMidLine(basePt, bevelMidPt);

	// finally the bevel segment endpoints are computed as offsets from
	// the mitre midline
	Coordinate bevelEndLeft;
	mitreMidLine.pointAlongOffset(1.0, bevelHalfLen, bevelEndLeft);
	Coordinate bevelEndRight;
	mitreMidLine.pointAlongOffset(1.0, -bevelHalfLen, bevelEndRight);

	if (side == Position::LEFT) {
		vertexList.addPt(bevelEndLeft);
		vertexList.addPt(bevelEndRight);
	}
	else {
		vertexList.addPt(bevelEndRight);
		vertexList.addPt(bevelEndLeft);
	}

}

/* private */
void
OffsetCurveBuilder::addBevelJoin( const geom::LineSegment& offset0,
	                  const geom::LineSegment& offset1)
{
	vertexList.addPt(offset0.p1);
	vertexList.addPt(offset1.p0);
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.34  2006/03/27 17:59:00  strk
 * Fixed small leak.
 *
 * Revision 1.33  2006/03/27 17:04:18  strk
 * Cleanups and explicit initializations
 *
 * Revision 1.32  2006/03/20 11:42:29  strk
 * Added missing <cmath> include
 *
 * Revision 1.31  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 * Revision 1.30  2006/03/11 16:58:41  strk
 * Fixed bug in OffsetCurveBuilder::getCoordinates.
 *
 * Revision 1.29  2006/03/09 17:40:24  strk
 * Fixed bug#33 (hopefully)
 *
 * Revision 1.28  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.27  2006/03/07 14:20:15  strk
 * Big deal of heap allocations reduction
 *
 * Revision 1.26  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.25  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.24  2006/02/28 19:22:21  strk
 * Fixed in-place definition of static members in OffsetCurveBuilder (bug#33)
 *
 * Revision 1.23  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.22  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.21  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.20  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.19  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.18  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.17  2005/02/17 09:56:31  strk
 * Commented out unused variable.
 *
 * Revision 1.16  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.15  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.14  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.13  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.12  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/27 08:37:16  strk
 * Fixed a bug preventing OffsetCurveBuilder point list from being reset.
 *
 * Revision 1.9  2004/05/26 19:48:19  strk
 * Changed abs() to fabs() when working with doubles.
 * Used dynamic_cast<> instead of typeid() when JTS uses instanceof.
 *
 * Revision 1.8  2004/05/19 13:40:49  strk
 * Fixed bug in ::addCircle
 *
 * Revision 1.7  2004/05/05 13:08:01  strk
 * Leaks fixed, explicit allocations/deallocations reduced.
 *
 * Revision 1.6  2004/04/20 10:58:04  strk
 * More memory leaks removed.
 *
 * Revision 1.5  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.4  2004/04/19 15:14:46  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.3  2004/04/16 13:03:17  strk
 * More leaks fixed
 *
 * Revision 1.2  2004/04/16 12:48:07  strk
 * Leak fixes.
 *
 * Revision 1.1  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

