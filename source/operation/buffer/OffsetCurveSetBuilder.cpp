/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
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


#include "../../headers/opBuffer.h"
#include <typeinfo>

namespace geos {

OffsetCurveSetBuilder::OffsetCurveSetBuilder(const Geometry *newInputGeom, double newDistance, OffsetCurveBuilder *newCurveBuilder){
	cga=new RobustCGAlgorithms();
	curveList=new vector<SegmentString*>();
	inputGeom=newInputGeom;
	distance=newDistance;
	curveBuilder=newCurveBuilder;
}

OffsetCurveSetBuilder::~OffsetCurveSetBuilder(){
	delete cga;
	for (int i=0; i<curveList->size(); i++)
	{
		delete (*curveList)[i];
	}
	delete curveList;
}
/**
* Computes the set of raw offset curves for the buffer.
* Each offset curve has an attached {@link Label} indicating
* its left and right location.
*
* @return a Collection of SegmentStrings representing the raw buffer curves
*/
vector<SegmentString*>* OffsetCurveSetBuilder::getCurves(){
	add(inputGeom);
	return curveList;
}

void OffsetCurveSetBuilder::addCurves(vector<CoordinateList*> *lineList, int leftLoc, int rightLoc){
	for (int i=0;i<(int)lineList->size();i++) {
		CoordinateList *coords=(*lineList)[i];
		addCurve(coords, leftLoc, rightLoc);
	}
}

/**
* Creates a {@link SegmentString} for a coordinate list which is a raw offset curve,
* and adds it to the list of buffer curves.
* The SegmentString is tagged with a Label giving the topology of the curve.
* The curve may be oriented in either direction.
* If the curve is oriented CW, the locations will be:
* <br>Left: Location.EXTERIOR
* <br>Right: Location.INTERIOR
*/
void OffsetCurveSetBuilder::addCurve(const CoordinateList *coord, int leftLoc, int rightLoc){
	// don't add null curves!
	if (coord->getSize() < 2) return;
	// add the edge for a coordinate list which is a raw offset curve
	Label *newlabel = new Label(0, Location::BOUNDARY, leftLoc, rightLoc);
	SegmentString *e=new SegmentString(coord,newlabel);
	curveList->push_back(e);
}


void
OffsetCurveSetBuilder::add(const Geometry *g)
{
	if (g->isEmpty()) return;
	if (typeid(*g)==typeid(Polygon))
		addPolygon((Polygon*) g);
	else if (typeid(*g)==typeid(LineString))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(LinearRing))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(Point))
		addPoint((Point*) g);
	else if (typeid(*g)==typeid(MultiPoint))
		addCollection((MultiPoint*) g);
	else if (typeid(*g)==typeid(MultiLineString))
		addCollection((MultiLineString*) g);
	else if (typeid(*g)==typeid(MultiPolygon))
		addCollection((MultiPolygon*) g);
	else if (typeid(*g)==typeid(GeometryCollection))
		addCollection((GeometryCollection*) g);
	else {
		string out=typeid(*g).name();
		throw new UnsupportedOperationException("GeometryGraph::add(Geometry *): unknown geometry type: "+out);
	}
}

void OffsetCurveSetBuilder::addCollection(const GeometryCollection *gc){
	for (int i=0;i<gc->getNumGeometries(); i++) {
		const Geometry *g=gc->getGeometryN(i);
		add(g);
	}
}
/**
* Add a Point to the graph->
*/
void OffsetCurveSetBuilder::addPoint(const Point *p){
	if (distance <= 0.0) return;
	CoordinateList *coord=p->getCoordinates();
	vector<CoordinateList*> *lineList=curveBuilder->getLineCurve(coord, distance);
	delete coord;
	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
	delete lineList;
}

void OffsetCurveSetBuilder::addLineString(const LineString *line){
	if (distance <= 0.0) return;
	CoordinateList *coord=CoordinateList::removeRepeatedPoints(line->getCoordinates());
	vector<CoordinateList*> *lineList=curveBuilder->getLineCurve(coord, distance);
	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
}

void
OffsetCurveSetBuilder::addPolygon(const Polygon *p)
{
	double offsetDistance=distance;
	int offsetSide=Position::LEFT;
	if (distance < 0.0) {
		offsetDistance=-distance;
		offsetSide=Position::RIGHT;
	}
	const LinearRing *shell=(const LinearRing *)p->getExteriorRing();
	CoordinateList *shellCoord=CoordinateList::removeRepeatedPoints(shell->getCoordinatesRO());
	// optimization - don't bother computing buffer
	// if the polygon would be completely eroded
	if (distance < 0.0 && isErodedCompletely(shellCoord, distance))
		return;
	addPolygonRing(shellCoord,offsetDistance,offsetSide,Location::EXTERIOR,Location::INTERIOR);
	delete shellCoord;
	for (int i=0;i<p->getNumInteriorRing(); i++) {
		const LinearRing *hole=(const LinearRing *)p->getInteriorRingN(i);
		CoordinateList *holeCoord=CoordinateList::removeRepeatedPoints(hole->getCoordinatesRO());
		// optimization - don't bother computing buffer for this hole
		// if the hole would be completely covered
		if (distance > 0.0 && isErodedCompletely(holeCoord, -distance))
			return;
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CCW)
		addPolygonRing(holeCoord,offsetDistance,Position::opposite(offsetSide),Location::INTERIOR,Location::EXTERIOR);
	}
}

/**
* Add an offset curve for a ring->
* The side and left and right topological location arguments
* assume that the ring is oriented CW->
* If the ring is in the opposite orientation,
* the left and right locations must be interchanged and the side flipped->
*
* @param coord the coordinates of the ring (must not contain repeated points)
* @param offsetDistance the distance at which to create the buffer
* @param side the side of the ring on which to construct the buffer line
* @param cwLeftLoc the location on the L side of the ring (if it is CW)
* @param cwRightLoc the location on the R side of the ring (if it is CW)
*/
void OffsetCurveSetBuilder::addPolygonRing(const CoordinateList *coord, double offsetDistance, int side, int cwLeftLoc, int cwRightLoc){
	//Coordinate[] coord=CoordinateArrays->removeRepeatedPoints(lr->getCoordinates());
	int leftLoc =cwLeftLoc;
	int rightLoc=cwRightLoc;
	if (cga->isCCW(coord)) {
		leftLoc=cwRightLoc;
		rightLoc=cwLeftLoc;
		side=Position::opposite(side);
	}
	vector<CoordinateList*> *lineList=curveBuilder->getRingCurve(coord, side, offsetDistance);
	addCurves(lineList, leftLoc, rightLoc);
}

/**
* The ringCoord is assumed to contain no repeated points->
* It may be degenerate (i->e-> contain only 1, 2, or 3 points)->
* In this case it has no area, and hence has a minimum diameter of 0->
*
* @param ringCoord
* @param offsetDistance
* @return
*/
bool OffsetCurveSetBuilder::isErodedCompletely(CoordinateList *ringCoord, double bufferDistance){
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
	* It is based on the fact that the minimum diameter of the ring pointset
	* provides an upper bound on the buffer distance which would erode the
	* ring->
	* If the buffer distance is less than the minimum diameter, the ring
	* may still be eroded, but this will be determined by
	* a full topological computation->
	*
	*/
	LinearRing *ring=inputGeom->getFactory()->createLinearRing(ringCoord);
	MinimumDiameter *md=new MinimumDiameter(ring);
	minDiam=md->getLength();
	delete ring;
	delete md;
	//System->out->println(md->getDiameter());
	return minDiam < 2 * abs(bufferDistance);
}

/**
* Tests whether a triangular ring would be eroded completely by the given
* buffer distance->
* This is a precise test->  It uses the fact that the inner buffer of a
* triangle converges on the inCentre of the triangle (the point
* equidistant from all sides)->  If the buffer distance is greater than the
* distance of the inCentre from a side, the triangle will be eroded completely->
*
* This test is important, since it removes a problematic case where
* the buffer distance is slightly larger than the inCentre distance->
* In this case the triangle buffer curve "inverts" with incorrect topology,
* producing an incorrect hole in the buffer->
*
* @param triangleCoord
* @param bufferDistance
* @return
*/
bool OffsetCurveSetBuilder::isTriangleErodedCompletely(CoordinateList *triangleCoord,double bufferDistance){
	Triangle *tri=new Triangle(triangleCoord->getAt(0), triangleCoord->getAt(1), triangleCoord->getAt(2));
	Coordinate *inCentre=tri->inCentre();
	double distToCentre=cga->distancePointLine(*inCentre, tri->p0, tri->p1);
	return distToCentre < abs(bufferDistance);
}



}
