/**********************************************************************
 * $Id: LineSegment.cpp 3257 2011-03-02 12:46:57Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009 2011 Sandro Santilli <strk@keybit.net>
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
 * Last port: geom/LineSegment.java r18 (JTS-1.11)
 *
 **********************************************************************/

#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h> // for toGeometry
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h> 
#include <geos/geom/CoordinateArraySequence.h> // should we really be using this?
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/HCoordinate.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/util/IllegalStateException.h>
#include <geos/profiler.h>
#include <geos/inline.h>

#include <sstream>
#include <cmath>

#ifndef GEOS_INLINE
# include <geos/geom/LineSegment.inl>
#endif

using namespace std;
//using namespace geos::algorithm;
using geos::algorithm::HCoordinate;
using geos::algorithm::NotRepresentableException;
using geos::algorithm::LineIntersector;

namespace geos {
namespace geom { // geos::geom


/*public*/
void
LineSegment::reverse()
{
	// TODO: use std::swap<>
	Coordinate temp=p0;
	p0=p1;
	p1=temp;
}

/*public*/
double
LineSegment::projectionFactor(const Coordinate& p) const
{
	if (p==p0) return 0.0;
	if (p==p1) return 1.0;
    // Otherwise, use comp.graphics.algorithms Frequently Asked Questions method
    /*(1)     	      AC dot AB
                   r = ---------
                         ||AB||^2
                r has the following meaning:
                r=0 P = A
                r=1 P = B
                r<0 P is on the backward extension of AB
                r>1 P is on the forward extension of AB
                0<r<1 P is interior to AB
        */
	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	double len2=dx*dx+dy*dy;
	double r=((p.x-p0.x)*dx+(p.y-p0.y)*dy)/len2;
	return r;
}

/*public*/
double
LineSegment::segmentFraction(const Coordinate& inputPt) const
{
	double segFrac = projectionFactor(inputPt);
	if (segFrac < 0.0)
		segFrac = 0.0;
	else if (segFrac > 1.0)
		segFrac = 1.0;
	return segFrac;
}

/*public*/
void
LineSegment::project(const Coordinate& p, Coordinate& ret) const
{
	if (p==p0 || p==p1) ret=p;
	double r=projectionFactor(p);
	ret=Coordinate(p0.x+r*(p1.x-p0.x),p0.y+r*(p1.y-p0.y));
}

bool
LineSegment::project(const LineSegment& seg, LineSegment& ret) const
{
	double pf0=projectionFactor(seg.p0);
	double pf1=projectionFactor(seg.p1);
	// check if segment projects at all

	if (pf0>=1.0 && pf1>=1.0) return false;

	if (pf0<=0.0 && pf1<=0.0) return false;

	Coordinate newp0;
	project(seg.p0, newp0);
	Coordinate newp1;
	project(seg.p1, newp1);

	ret.setCoordinates(newp0, newp1);

	return true;
}

//Coordinate*
void
LineSegment::closestPoint(const Coordinate& p, Coordinate& ret) const
{
	double factor=projectionFactor(p);
	if (factor>0 && factor<1) {
		project(p, ret);
		return;
	}
	double dist0=p0.distance(p);
	double dist1=p1.distance(p);
	if (dist0<dist1)
	{
		ret=p0;
		return;
	}
	ret=p1;
}

/*public*/
int
LineSegment::compareTo(const LineSegment& other) const
{
	int comp0=p0.compareTo(other.p0);
	if (comp0!=0) return comp0;
	return p1.compareTo(other.p1);
}

/*public*/
bool
LineSegment::equalsTopo(const LineSegment& other) const
{
	return (p0==other.p0 && p1==other.p1) || (p0==other.p1 && p1==other.p0);
}

/*public*/
int
LineSegment::orientationIndex(const LineSegment& seg) const
{
	int orient0 = algorithm::CGAlgorithms::orientationIndex(p0, p1, seg.p0);
	int orient1 = algorithm::CGAlgorithms::orientationIndex(p0, p1, seg.p1);
	// this handles the case where the points are L or collinear
	if (orient0 >= 0 && orient1 >= 0)
		return max(orient0, orient1);
	// this handles the case where the points are R or collinear
	if (orient0 <= 0 && orient1 <= 0)
		return max(orient0, orient1);
	// points lie on opposite sides ==> indeterminate orientation
	return 0;
}

CoordinateSequence*
LineSegment::closestPoints(const LineSegment& line)
{
	// test for intersection
	Coordinate intPt;
	if ( intersection(line, intPt) )
	{
		CoordinateSequence *cl=new CoordinateArraySequence(new vector<Coordinate>(2, intPt));
		return cl;
	}

	/*
	 * if no intersection closest pair contains at least one endpoint.
	 * Test each endpoint in turn.
	 */
	CoordinateSequence *closestPt=new CoordinateArraySequence(2);
	//vector<Coordinate> *cv = new vector<Coordinate>(2);

	double minDistance=DoubleMax;
	double dist;

	Coordinate close00;
	closestPoint(line.p0, close00);
	minDistance = close00.distance(line.p0);

	closestPt->setAt(close00, 0);
	closestPt->setAt(line.p0, 1);

	Coordinate close01;
	closestPoint(line.p1, close01);
	dist = close01.distance(line.p1);
	if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(close01,0);
		closestPt->setAt(line.p1,1);
		//(*cv)[0] = close01;
		//(*cv)[1] = line.p1; 
	}

	Coordinate close10;
	line.closestPoint(p0, close10);
	dist = close10.distance(p0);
		if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(p0,0);
		closestPt->setAt(close10,1);
		//(*cv)[0] = p0;
		//(*cv)[1] = close10;
	}

	Coordinate close11;
	line.closestPoint(p1, close11);
	dist = close11.distance(p1);
	if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(p1,0);
		closestPt->setAt(close11,1);
		//(*cv)[0] = p1;
		//(*cv)[1] = *close11;
	}

	return closestPt;
}

bool
LineSegment::intersection(const LineSegment& line, Coordinate& ret) const
{
	algorithm::LineIntersector li;
	li.computeIntersection(p0, p1, line.p0, line.p1);
	if (li.hasIntersection()) {
		ret=li.getIntersection(0);
		return true;
	}
	return false;
}

bool
LineSegment::lineIntersection(const LineSegment& line, Coordinate& ret) const
{
	try {
		HCoordinate::intersection(p0, p1, line.p0, line.p1, ret);
		return true;
	}
	catch (const NotRepresentableException& /*ex*/) {
		// eat this exception, and return null;
	}
	return false;
}


/* public */
void
LineSegment::pointAlongOffset(double segmentLengthFraction,
	                      double offsetDistance,
	                      Coordinate& ret) const
{
	// the point on the segment line
	double segx = p0.x + segmentLengthFraction * (p1.x - p0.x);
	double segy = p0.y + segmentLengthFraction * (p1.y - p0.y);

	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;
	double len = sqrt(dx * dx + dy * dy);

	double ux = 0.0;
	double uy = 0.0;
	if (offsetDistance != 0.0) {
		if (len <= 0.0) {
			throw util::IllegalStateException("Cannot compute offset from zero-length line segment");
		}

		// u is the vector that is the length of the offset,
		// in the direction of the segment
		ux = offsetDistance * dx / len;
		uy = offsetDistance * dy / len;
	}

	// the offset point is the seg point plus the offset
	// vector rotated 90 degrees CCW
	double offsetx = segx - uy;
	double offsety = segy + ux;

	ret = Coordinate(offsetx, offsety);
}

/* public */
std::auto_ptr<LineString>
LineSegment::toGeometry(const GeometryFactory& gf) const
{
	CoordinateSequence *cl=new CoordinateArraySequence();
	cl->add(p0);
	cl->add(p1);
	return std::auto_ptr<LineString>(
		gf.createLineString(cl) // ownership transferred
	);
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.36  2006/04/10 12:05:35  strk
 * Added inline-replicator implementation files to make sure
 * functions in .inl files are still available out-of-line.
 * A side effect is this should fix MingW build.
 *
 * Revision 1.35  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.34  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 * Revision 1.33  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.32  2006/03/16 13:28:21  strk
 * obsoleted getCoordinate(), replaced by operator[]
 *
 * Revision 1.31  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.30  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.29  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.28  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.27  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.26  2006/03/01 17:16:38  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.25  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.24  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.23  2006/01/31 19:07:33  strk
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
 * Revision 1.22  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.21  2005/06/24 11:09:42  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.20  2005/05/23 15:04:10  strk
 * Fixed bug in ::reverse()
 *
 * Revision 1.19  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.18  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
 * Revision 1.17  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.16  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.15  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.14  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.13  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.12  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.11  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.9  2003/10/11 01:56:08  strk
 * Code base padded with 'const' keywords ;)
 *
 **********************************************************************/

