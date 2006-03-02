/**********************************************************************
 * $Id$
 *
 * GEOS-Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev. 1.7 
 *
 **********************************************************************/

#include <cassert>
#include <geos/opBuffer.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::geomgraph;
using namespace geos::noding;
using namespace geos::algorithm;
using namespace geos::operation::overlay;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

const double OffsetCurveBuilder::PI_OVER_2 = 1.570796326794895;
const double OffsetCurveBuilder::MAX_CLOSING_SEG_LEN = 3.0;

OffsetCurveBuilder::OffsetCurveBuilder(const PrecisionModel *newPrecisionModel):
	li(new LineIntersector()),
	maxCurveSegmentError(0.0),
	ptList(new CoordinateArraySequence()),
	distance(0.0),
	precisionModel(newPrecisionModel),
	endCapStyle(BufferOp::CAP_ROUND),
	seg0(new LineSegment()),
	seg1(new LineSegment()),
	offset0(new LineSegment()),
	offset1(new LineSegment())
{
	int limitedQuadSegs=DEFAULT_QUADRANT_SEGMENTS<1 ? 1 : DEFAULT_QUADRANT_SEGMENTS;
	filletAngleQuantum=3.14159265358979 / 2.0 / limitedQuadSegs;
}

OffsetCurveBuilder::OffsetCurveBuilder(const PrecisionModel *newPrecisionModel,
		int quadrantSegments):
	li(new LineIntersector()),
	maxCurveSegmentError(0.0),
	ptList(new CoordinateArraySequence()),
	distance(0.0),
	precisionModel(newPrecisionModel),
	endCapStyle(BufferOp::CAP_ROUND),
	seg0(new LineSegment()),
	seg1(new LineSegment()),
	offset0(new LineSegment()),
	offset1(new LineSegment())
{
	int limitedQuadSegs=quadrantSegments<1 ? 1 : quadrantSegments;
	filletAngleQuantum=3.14159265358979  / 2.0 / limitedQuadSegs;
}

OffsetCurveBuilder::~OffsetCurveBuilder()
{
	delete li;
	delete seg0;
	delete seg1;
	delete offset0;
	delete offset1;
	delete ptList;
	for (unsigned int i=0; i<ptLists.size(); i++) delete ptLists[i];
}


/*public*/
void
OffsetCurveBuilder::getLineCurve(const CoordinateSequence *inputPts,
		double distance, vector<CoordinateSequence*>& lineList)
{
	// a zero or negative width buffer of a line/point is empty
	if (distance<= 0.0) return;

	init(distance);

	if (inputPts->getSize() < 2) {
		switch (endCapStyle) {
			case BufferOp::CAP_ROUND:
				addCircle(inputPts->getAt(0), distance);
				break;
			case BufferOp::CAP_SQUARE:
				addSquare(inputPts->getAt(0), distance);
				break;
			// default is for buffer to be empty (e.g. for a butt line cap);
		}
	} else {
		computeLineBufferCurve(inputPts);
	}
	CoordinateSequence *lineCoord=getCoordinates();
	lineList.push_back(lineCoord);
}

/*public*/
void
OffsetCurveBuilder::getRingCurve(const CoordinateSequence *inputPts,
		int side, double distance,
		vector<CoordinateSequence*>& lineList)
{
	init(distance);
	if (inputPts->getSize()<= 2)
	{
		getLineCurve(inputPts, distance, lineList);
		return;
	}
	// optimize creating ring for for zero distance
	if (distance==0.0) {
		ptLists.push_back(ptList);
		ptList = inputPts->clone(); 
		lineList.push_back(ptList);
		return;
	}
	computeRingBufferCurve(inputPts, side);
	lineList.push_back(getCoordinates()); // this will be ptList
}

void
OffsetCurveBuilder::init(double newDistance)
{
	distance=newDistance;
	maxCurveSegmentError=distance*(1-cos(filletAngleQuantum/2.0));
	// Point list needs to be reset
	// but if a previous point list exists
	// we'd better back it up for final deletion
	ptLists.push_back(ptList);
	ptList=new CoordinateArraySequence();
}

// returns ptList / is private / might just be avoided !
CoordinateSequence*
OffsetCurveBuilder::getCoordinates()
{
	// check that points are a ring,
	// add the startpoint again if they are not
	if (ptList->getSize()>1) {
		const Coordinate &start=ptList->getAt(0);
		const Coordinate &end=ptList->getAt(1);
#if GEOS_DEBUG
		cerr << __FILE__ << ":" << __LINE__
			<< ":"
			<< " start:" << start
			<< " end:" << end << endl;
#endif
		if (!(start==end)) addPt(start);
	}
	return ptList;
}

void
OffsetCurveBuilder::computeLineBufferCurve(const CoordinateSequence *inputPts)
{
	int n=inputPts->getSize()-1;
	// compute points for left side of line
	initSideSegments(inputPts->getAt(0),inputPts->getAt(1), Position::LEFT);
	for (int i=2;i<= n;i++) {
		addNextSegment(inputPts->getAt(i), true);
	}
	addLastSegment();
	// add line cap for end of line
	addLineEndCap(inputPts->getAt(n-1),inputPts->getAt(n));
	// compute points for right side of line
	initSideSegments(inputPts->getAt(n),inputPts->getAt(n-1),Position::LEFT);
	for (int i=n-2;i>= 0;i--) {
		addNextSegment(inputPts->getAt(i), true);
	}
	addLastSegment();
	// add line cap for start of line
	addLineEndCap(inputPts->getAt(1),inputPts->getAt(0));
	closePts();
}

void
OffsetCurveBuilder::computeRingBufferCurve(const CoordinateSequence *inputPts,
	int side)
{
	int n=inputPts->getSize()-1;
	initSideSegments(inputPts->getAt(n-1),inputPts->getAt(0), side);
	for (int i=1;i<= n;i++) {
		bool addStartPoint=i != 1;
		addNextSegment(inputPts->getAt(i),addStartPoint);
	}
	closePts();
}

/*private*/
void
OffsetCurveBuilder::addPt(const Coordinate &pt)
{
	Coordinate bufPt=pt;
	assert(precisionModel);
	precisionModel->makePrecise(bufPt);

	// don't add duplicate points

	unsigned int npts=ptList->getSize();
	if ( npts ) {
		const Coordinate& lastPt=ptList->getAt(npts-1);
#if GEOS_DEBUG
		cerr<<__FUNCTION__<<" lastPt:"<<lastPt<<", currPt:"<<bufPt<<endl;
#endif
		if ( bufPt.equals2D(lastPt) ) {
#if GEOS_DEBUG
			cerr<<" they equal!"<<endl;
#endif
			return;
		}
	}

	ptList->add(bufPt);
}

void
OffsetCurveBuilder::closePts()
{

	int ptsize=ptList->getSize();

	if (ptsize<1) return;

	const Coordinate &startPt=ptList->getAt(0);
	const Coordinate &lastPt=ptList->getAt(ptsize-1);

	if (startPt==lastPt) return;

	ptList->add(startPt);
}

void
OffsetCurveBuilder::initSideSegments(const Coordinate &nS1, const Coordinate &nS2, int nSide)
{
	s1=nS1;
	s2=nS2;
	side=nSide;
	seg1->setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);
}

void
OffsetCurveBuilder::addNextSegment(const Coordinate &p, bool addStartPoint)
{
	// s0-s1-s2 are the coordinates of the previous segment and the current one
	s0=s1;
	s1=s2;
	s2=p;
	seg0->setCoordinates(s0, s1);
	computeOffsetSegment(seg0, side, distance, offset0);
	seg1->setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);

	// do nothing if points are equal
	if (s1==s2) return;
	int orientation=CGAlgorithms::computeOrientation(s0, s1, s2);
	bool outsideTurn =(orientation==CGAlgorithms::CLOCKWISE
						&& side==Position::LEFT)
						||(orientation==CGAlgorithms::COUNTERCLOCKWISE 
						&& side==Position::RIGHT);
	if (orientation==0) { // lines are collinear
		li->computeIntersection(s0,s1,s1,s2);
		int numInt=li->getIntersectionNum();
		/**
		* if numInt is<2, the lines are parallel and in the same direction.
		* In this case the point can be ignored, since the offset lines will also be
		* parallel.
		*/
		if (numInt>= 2) {
			/**
			* segments are collinear but reversing.  Have to add an "end-cap" fillet
			* all the way around to other direction
			* This case should ONLY happen for LineStrings, so the orientation is always CW.
			* (Polygons can never have two consecutive segments which are parallel but reversed,
			* because that would be a self intersection.
			*/
			addFillet(s1, offset0->p1, offset1->p0, CGAlgorithms::CLOCKWISE, distance);
		}
	} else if (outsideTurn) {
		// add a fillet to connect the endpoints of the offset segments
		if (addStartPoint) addPt(offset0->p1);
		// TESTING-comment out to produce beveled joins
		addFillet(s1, offset0->p1, offset1->p0, orientation, distance);
		addPt(offset1->p0);
	} else { // inside turn
		/**
		* add intersection point of offset segments (if any)
		*/
		li->computeIntersection( offset0->p0, offset0->p1,offset1->p0, offset1->p1);
		if (li->hasIntersection()) {
			addPt(li->getIntersection(0));
		} else {
			/**
			* If no intersection, it means the angle is so small and/or the offset so large
			* that the offsets segments don't intersect.
			* In this case we must add a offset joining curve to make sure the buffer line
			* is continuous and tracks the buffer correctly around the corner.
			* Note that the joining curve won't appear in the final buffer.
			*
			* The intersection test above is vulnerable to robustness errors;
			* i.e. it may be that the offsets should intersect very close to their
			* endpoints, but don't due to rounding.  To handle this situation
			* appropriately, we use the following test:
			* If the offset points are very close, don't add a joining curve
			* but simply use one of the offset points
			*/
			if (offset0->p1.distance(offset1->p0)<distance / 1000.0) {
				addPt(offset0->p1);
			} else {
				// add endpoint of this segment offset
				addPt(offset0->p1);
				// <FIX> MD-add in centre point of corner, to make sure offset closer lines have correct topology
				addPt(s1);
				addPt(offset1->p0);
			}
		}
	}
}

/**
 * Add last offset point
 */
void
OffsetCurveBuilder::addLastSegment()
{
	addPt(offset1->p1);
}

/**
 * Compute an offset segment for an input segment on a given side and at
 * a given distance.
 * The offset points are computed in full double precision, for accuracy.
 *
 * @param seg the segment to offset
 * @param side the side of the segment the offset lies on
 * @param distance the offset distance
 * @param offset the points computed for the offset segment
 */
void
OffsetCurveBuilder::computeOffsetSegment(LineSegment *seg, int side,
	double distance, LineSegment *offset)
{
	int sideSign=side==Position::LEFT ? 1 : -1;
	double dx=seg->p1.x-seg->p0.x;
	double dy=seg->p1.y-seg->p0.y;
	double len=sqrt(dx*dx+dy*dy);
	// u is the vector that is the length of the offset, in the direction of the segment
	double ux=sideSign*distance*dx / len;
	double uy=sideSign*distance*dy / len;
	offset->p0.x=seg->p0.x-uy;
	offset->p0.y=seg->p0.y+ux;
	offset->p1.x=seg->p1.x-uy;
	offset->p1.y=seg->p1.y+ux;
}

/**
 * Add an end cap around point p1, terminating a line segment coming from p0
 */
void
OffsetCurveBuilder::addLineEndCap(const Coordinate &p0,const Coordinate &p1)
{
	LineSegment *seg=new LineSegment(p0, p1);
	LineSegment *offsetL=new LineSegment();
	computeOffsetSegment(seg, Position::LEFT, distance, offsetL);
	LineSegment *offsetR=new LineSegment();
	computeOffsetSegment(seg, Position::RIGHT, distance, offsetR);
	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	double angle=atan2(dy, dx);
	switch (endCapStyle) {
		case BufferOp::CAP_ROUND:
			// add offset seg points with a fillet between them
			addPt(offsetL->p1);
			addFillet(p1, angle+PI_OVER_2,angle-PI_OVER_2, CGAlgorithms::CLOCKWISE, distance);
			addPt(offsetR->p1);
			break;
		case BufferOp::CAP_BUTT:
			// only offset segment points are added
			addPt(offsetL->p1);
			addPt(offsetR->p1);
			break;
		case BufferOp::CAP_SQUARE:
			// add a square defined by extensions of the offset segment endpoints
			Coordinate *squareCapSideOffset=new Coordinate();
			squareCapSideOffset->x=fabs(distance)*cos(angle);
			squareCapSideOffset->y=fabs(distance)*sin(angle);
			Coordinate *squareCapLOffset=new Coordinate(offsetL->p1.x+squareCapSideOffset->x, offsetL->p1.y+squareCapSideOffset->y);
			Coordinate *squareCapROffset=new Coordinate(offsetR->p1.x+squareCapSideOffset->x, offsetR->p1.y+squareCapSideOffset->y);
			addPt(*squareCapLOffset);
			addPt(*squareCapROffset);
			delete squareCapSideOffset;
			delete squareCapLOffset;
			delete squareCapROffset;
			break;
	}
	delete seg;
	delete offsetL;
	delete offsetR;
}

/**
 * @param p base point of curve
 * @param p0 start point of fillet curve
 * @param p1 endpoint of fillet curve
 */
void
OffsetCurveBuilder::addFillet(const Coordinate &p, const Coordinate &p0,
	const Coordinate &p1, int direction, double distance)
{
	double dx0=p0.x-p.x;
	double dy0=p0.y-p.y;
	double startAngle=atan2(dy0, dx0);
	double dx1=p1.x-p.x;
	double dy1=p1.y-p.y;
	double endAngle=atan2(dy1, dx1);
	if (direction==CGAlgorithms::CLOCKWISE) {
		if (startAngle<= endAngle) startAngle += 2.0*3.1415926535;
	} else {    // direction==COUNTERCLOCKWISE
		if (startAngle>=endAngle) startAngle-=2.0*3.1415926535;
	}
	addPt(p0);
	addFillet(p, startAngle, endAngle, direction, distance);
	addPt(p1);
}

/**
 * Adds points for a fillet.
 * The start and end point for the fillet are not added,
 * the caller must add them if required.
 *
 * @param direction is -1 for a CW angle, 1 for a CCW angle
 */
void
OffsetCurveBuilder::addFillet(const Coordinate &p, double startAngle,
	double endAngle, int direction, double distance)
{
	int directionFactor=direction==CGAlgorithms::CLOCKWISE ? -1 : 1;
	double totalAngle=fabs(startAngle-endAngle);
	int nSegs=(int) (totalAngle / filletAngleQuantum+0.5);
	if (nSegs<1) return;   // no segments because angle is less than increment-nothing to do!
	double initAngle, currAngleInc;
	// choose angle increment so that each segment has equal length
	initAngle=0.0;
	currAngleInc=totalAngle / nSegs;
	double currAngle=initAngle;
	Coordinate pt;
	while (currAngle<totalAngle) {
		double angle=startAngle+directionFactor*currAngle;
		pt.x=p.x+distance*cos(angle);
		pt.y=p.y+distance*sin(angle);
		addPt(pt);
		currAngle += currAngleInc;
	}
}


/**
 * Adds a CW circle around a point
 */
void
OffsetCurveBuilder::addCircle(const Coordinate &p, double distance)
{
	// add start point
	Coordinate pt(p);
	pt.x+=distance;
	addPt(pt);
	addFillet(p, 0.0, 2.0*3.1415926535, -1, distance);
}

/**
 * Adds a CW square around a point
 */
void
OffsetCurveBuilder::addSquare(const Coordinate &p, double distance)
{
	// add start point
	addPt(*(new Coordinate(p.x+distance, p.y+distance)));
	addPt(*(new Coordinate(p.x+distance, p.y-distance)));
	addPt(*(new Coordinate(p.x-distance, p.y-distance)));
	addPt(*(new Coordinate(p.x-distance, p.y+distance)));
	addPt(*(new Coordinate(p.x+distance, p.y+distance)));
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
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

