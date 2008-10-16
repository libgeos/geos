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

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/MinimumDiameter.h>
#include <geos/util/UnsupportedOperationException.h>
#include <geos/operation/buffer/OffsetCurveSetBuilder.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Location.h>
#include <geos/geom/Triangle.h>
#include <geos/geomgraph/Position.h>
#include <geos/geomgraph/Label.h>
#include <geos/noding/SegmentString.h>

#include <cmath>
#include <vector>
#include <memory>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//using namespace geos::operation::overlay;
using namespace geos::geom;
using namespace geos::noding; // SegmentString
using namespace geos::geomgraph; // Label, Position
using namespace geos::algorithm; // CGAlgorithms

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
	for (size_t i=0, n=curveList.size(); i<n; ++i)
	{
		delete curveList[i]->getCoordinates();
		delete curveList[i];
	}
	for (size_t i=0, n=newLabels.size(); i<n; ++i)
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
	for (size_t i=0, n=lineList.size(); i<n; ++i)
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
		delete coord;
		return;
	}

	// add the edge for a coordinate list which is a raw offset curve
	Label *newlabel = new Label(0, Location::BOUNDARY, leftLoc, rightLoc);
	SegmentString *e=new SegmentString(coord, newlabel); // SegmentString doesnt own the sequence, so we need to delete in the destructor
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
	std::auto_ptr<CoordinateSequence> coord(CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO()));
#if GEOS_DEBUG
	std::cerr<<" After coordinate removal: "<<coord->toString()<<std::endl;
#endif
	std::vector<CoordinateSequence*> lineList;
	curveBuilder.getLineCurve(coord.get(), distance, lineList);
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

	// FIXME: avoid the C-style cast
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

	for (size_t i=0, n=p->getNumInteriorRing(); i<n; ++i)
	{
		const LineString *hls=p->getInteriorRingN(i);
		assert(dynamic_cast<const LinearRing *>(hls));
		const LinearRing *hole=static_cast<const LinearRing *>(hls);
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
 * Revision 1.36  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.35  2006/06/09 07:42:13  strk
 * * source/geomgraph/GeometryGraph.cpp, source/operation/buffer/OffsetCurveSetBuilder.cpp, source/operation/overlay/OverlayOp.cpp, source/operation/valid/RepeatedPointTester.cpp: Fixed warning after Polygon ring accessor methods changed to work with size_t. Small optimizations in loops.
 *
 * Revision 1.34  2006/05/03 09:14:22  strk
 * * source/operation/buffer/OffsetCurveSetBuilder.cpp: used auto_ptr to protect leaks of CoordinateSequence
 * * source/noding/ScaledNoder.cpp, source/headers/geos/noding/ScaledNoder.h: ported JTS bugfix in scale method.
 *
 * Revision 1.33  2006/03/27 17:04:18  strk
 * Cleanups and explicit initializations
 *
 * Revision 1.32  2006/03/22 11:18:39  strk
 * Changed back 'unable to find edge to compute depths' from assertion to TopologyException
 *
 **********************************************************************/

