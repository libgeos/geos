/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
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
 * Last port: operation/buffer/OffsetCurveSetBuilder.java rev. 1.7 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/opBuffer.h>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//using namespace std;
using namespace geos::geomgraph;
using namespace geos::noding;
using namespace geos::algorithm;
using namespace geos::operation::overlay;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

OffsetCurveSetBuilder::OffsetCurveSetBuilder(const Geometry& newInputGeom,
		double newDistance, OffsetCurveBuilder& newCurveBuilder):
	inputGeom(newInputGeom),
	distance(newDistance),
	curveBuilder(newCurveBuilder),
	curveList()
{
}

OffsetCurveSetBuilder::~OffsetCurveSetBuilder()
{
	//delete cga;
	for (unsigned int i=0, n=curveList.size(); i<n; ++i)
		delete curveList[i];
	for (unsigned int i=0, n=newLabels.size(); i<n; ++i)
		delete newLabels[i];
}

/* public */
std::vector<SegmentString*>&
OffsetCurveSetBuilder::getCurves()
{
	add(inputGeom);
	return curveList;
}

/*public*/
void
OffsetCurveSetBuilder::addCurves(const std::vector<CoordinateSequence*>& lineList,
	int leftLoc, int rightLoc)
{
	for (unsigned int i=0, n=lineList.size(); i<n; ++i)
	{
		CoordinateSequence *coords=lineList[i];
		addCurve(coords, leftLoc, rightLoc);
	}
}

/*private*/
void
OffsetCurveSetBuilder::addCurve(CoordinateSequence *coord,
	int leftLoc, int rightLoc)
{
#if GEOS_DEBUG
	std::cerr<<__FUNCTION__<<": coords="<<coord->toString()<<std::endl;
#endif
	// don't add null curves!
	if (coord->getSize() < 2) {
#if GEOS_DEBUG
		std::cerr<<" skipped (size<2)"<<std::endl;
#endif
		return;
	}

	// add the edge for a coordinate list which is a raw offset curve
	Label *newlabel = new Label(0, Location::BOUNDARY, leftLoc, rightLoc);
	SegmentString *e=new SegmentString(coord, newlabel);
	newLabels.push_back(newlabel);
	curveList.push_back(e);
}


/*private*/
void
OffsetCurveSetBuilder::add(const Geometry& g)
{
	if (g.isEmpty()) return;

	const Polygon *poly = dynamic_cast<const Polygon *>(&g);
	if ( poly ) {
		addPolygon(poly);
		return;
	}

	const LineString *line = dynamic_cast<const LineString *>(&g);
	if ( line ) {
		addLineString(line);
		return;
	}

	const Point *point = dynamic_cast<const Point *>(&g);
	if ( point ) {
		addPoint(point);
		return;
	}

	const GeometryCollection *collection = dynamic_cast<const GeometryCollection *>(&g);
	if ( collection ) {
		addCollection(collection);
		return;
	}

	std::string out=typeid(g).name();
	throw util::UnsupportedOperationException("GeometryGraph::add(Geometry &): unknown geometry type: "+out);
}

/*private*/
void
OffsetCurveSetBuilder::addCollection(const GeometryCollection *gc)
{
	for (int i=0, n=gc->getNumGeometries(); i<n; i++) {
		const Geometry *g=gc->getGeometryN(i);
		add(*g);
	}
}

/*private*/
void
OffsetCurveSetBuilder::addPoint(const Point *p)
{
	if (distance <= 0.0) return;
	const CoordinateSequence *coord=p->getCoordinatesRO();
	std::vector<CoordinateSequence*> lineList;
	curveBuilder.getLineCurve(coord, distance, lineList);

	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
	//delete lineList;
}

/*private*/
void
OffsetCurveSetBuilder::addLineString(const LineString *line)
{
	if (distance <= 0.0) return;
#if GEOS_DEBUG
	std::cerr<<__FUNCTION__<<": "<<line->toString()<<std::endl;
#endif
	CoordinateSequence *coord=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());
#if GEOS_DEBUG
	std::cerr<<" After coordinate removal: "<<coord->toString()<<std::endl;
#endif
	std::vector<CoordinateSequence*> lineList;
	curveBuilder.getLineCurve(coord, distance, lineList);
	delete coord;
	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
}

/*private*/
void
OffsetCurveSetBuilder::addPolygon(const Polygon *p)
{
	double offsetDistance=distance;

	int offsetSide=Position::LEFT;
	if (distance < 0.0)
	{
		offsetDistance = -distance;
		offsetSide = Position::RIGHT;
	}

	const LinearRing *shell=(const LinearRing *)p->getExteriorRing();
	CoordinateSequence *shellCoord = CoordinateSequence::removeRepeatedPoints(shell->getCoordinatesRO());

	// optimization - don't bother computing buffer
	// if the polygon would be completely eroded
	if (distance < 0.0 && isErodedCompletely(shellCoord, distance))
	{
		delete shellCoord;
		return;
	}

	addPolygonRing(
		shellCoord,
		offsetDistance,
		offsetSide,
		Location::EXTERIOR,
		Location::INTERIOR);

	delete shellCoord;

	for (int i=0;i<p->getNumInteriorRing(); i++)
	{
		const LinearRing *hole=(const LinearRing *)p->getInteriorRingN(i);
		CoordinateSequence *holeCoord=CoordinateSequence::removeRepeatedPoints(hole->getCoordinatesRO());

		// optimization - don't bother computing buffer for this hole
		// if the hole would be completely covered
		if (distance > 0.0 && isErodedCompletely(holeCoord, -distance))
		{
			delete holeCoord;
			continue;
		}

		// Holes are topologically labelled opposite to the shell,
		// since the interior of the polygon lies on their opposite
		// side (on the left, if the hole is oriented CCW)
		addPolygonRing(
			holeCoord,
			offsetDistance,
			Position::opposite(offsetSide),
			Location::INTERIOR,
			Location::EXTERIOR);

		delete holeCoord;
	}
}

/* private */
void
OffsetCurveSetBuilder::addPolygonRing(const CoordinateSequence *coord,
	double offsetDistance, int side, int cwLeftLoc, int cwRightLoc)
{
	int leftLoc=cwLeftLoc;
	int rightLoc=cwRightLoc;
#if GEOS_DEBUG
	std::cerr<<"OffsetCurveSetBuilder::addPolygonRing: CCW: "<<CGAlgorithms::isCCW(coord)<<std::endl;
#endif
	if (CGAlgorithms::isCCW(coord)) {
		leftLoc=cwRightLoc;
		rightLoc=cwLeftLoc;
#if GEOS_DEBUG
	std::cerr<<" side "<<side<<" becomes "<<Position::opposite(side)<<std::endl;
#endif
		side=Position::opposite(side);
	}
	std::vector<CoordinateSequence*> lineList;
	curveBuilder.getRingCurve(coord, side, offsetDistance, lineList);
	addCurves(lineList, leftLoc, rightLoc);
	//delete lineList;
}

/*private*/
bool
OffsetCurveSetBuilder::isErodedCompletely(CoordinateSequence *ringCoord,
	double bufferDistance)
{
	double minDiam=0.0;
	// degenerate ring has no area
	if (ringCoord->getSize() < 4)
		return bufferDistance < 0;
	// important test to eliminate inverted triangle bug
	// also optimizes erosion test for triangles
	if (ringCoord->getSize() == 4)
		return isTriangleErodedCompletely(ringCoord, bufferDistance);

	/**
	 * The following is a heuristic test to determine whether an
	 * inside buffer will be eroded completely->
	 * It is based on the fact that the minimum diameter of the ring
	 * pointset
	 * provides an upper bound on the buffer distance which would erode the
	 * ring->
	 * If the buffer distance is less than the minimum diameter, the ring
	 * may still be eroded, but this will be determined by
	 * a full topological computation->
	 *
	 */
	LinearRing *ring=inputGeom.getFactory()->createLinearRing(*ringCoord);
	MinimumDiameter md(ring); //=new MinimumDiameter(ring);
	minDiam=md.getLength();
	delete ring;

	//delete md;
	//System->out->println(md->getDiameter());
	return minDiam < (2 * fabs(bufferDistance));
}

/*private*/
bool
OffsetCurveSetBuilder::isTriangleErodedCompletely(
	CoordinateSequence *triangleCoord, double bufferDistance)
{
	Triangle tri(triangleCoord->getAt(0), triangleCoord->getAt(1), triangleCoord->getAt(2));

	Coordinate inCentre;
	tri.inCentre(inCentre);
	double distToCentre=CGAlgorithms::distancePointLine(inCentre, tri.p0, tri.p1);
	bool ret = distToCentre < fabs(bufferDistance);
	return ret;
}


} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.28  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.27  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.26  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.25  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.24  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.23  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.22  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.21  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
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
 * Revision 1.19  2005/05/20 16:14:52  strk
 * Fixed a bug in addPolygonRing setting wrong depths on Edges
 *
 * Revision 1.18  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.17  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.16  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.15  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.14  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.13  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.12  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.10  2004/05/26 19:48:19  strk
 * Changed abs() to fabs() when working with doubles.
 * Used dynamic_cast<> instead of typeid() when JTS uses instanceof.
 *
 * Revision 1.9  2004/05/19 09:57:54  ybychkov
 * Bugfix in OffsetCurveSetBuilder::addPolygon (JTS 1.4.1)
 *
 * Revision 1.8  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.7  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.6  2004/05/05 13:08:01  strk
 * Leaks fixed, explicit allocations/deallocations reduced.
 *
 * Revision 1.5  2004/04/20 10:58:04  strk
 * More memory leaks removed.
 *
 * Revision 1.4  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.3  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.2  2004/04/19 15:14:46  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.1  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

