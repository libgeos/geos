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
 * Revision 1.15  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.14  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
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


#include <geos/opBuffer.h>
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
	for (unsigned int i=0; i<curveList->size(); i++)
		delete (*curveList)[i];
	delete curveList;
	for (unsigned int i=0; i<newLabels.size(); i++)
		delete newLabels[i];
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

void
OffsetCurveSetBuilder::addCurves(const vector<CoordinateSequence*> *lineList, int leftLoc, int rightLoc)
{
	for (unsigned int i=0;i<lineList->size();i++) {
		const CoordinateSequence *coords=(*lineList)[i];
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
void
OffsetCurveSetBuilder::addCurve(const CoordinateSequence *coord, int leftLoc, int rightLoc)
{
	// don't add null curves!
	if (coord->getSize() < 2) return;
	// add the edge for a coordinate list which is a raw offset curve
	Label *newlabel = new Label(0, Location::BOUNDARY, leftLoc, rightLoc);
	SegmentString *e=new SegmentString(coord,newlabel);
	newLabels.push_back(newlabel);
	curveList->push_back(e);
}


void
OffsetCurveSetBuilder::add(const Geometry *g)
{
	if (g->isEmpty()) return;

	const Polygon *poly = dynamic_cast<const Polygon *>(g);
	if ( poly ) {
		addPolygon(poly);
		return;
	}

	const LineString *line = dynamic_cast<const LineString *>(g);
	if ( line ) {
		addLineString(line);
		return;
	}

	const Point *point = dynamic_cast<const Point *>(g);
	if ( point ) {
		addPoint(point);
		return;
	}

	const GeometryCollection *collection = dynamic_cast<const GeometryCollection *>(g);
	if ( collection ) {
		addCollection(collection);
		return;
	}

	string out=typeid(*g).name();
	throw new UnsupportedOperationException("GeometryGraph::add(Geometry *): unknown geometry type: "+out);
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
	CoordinateSequence *coord=p->getCoordinates();
	vector<CoordinateSequence*> *lineList=curveBuilder->getLineCurve(coord, distance);
	delete coord;
	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
	delete lineList;
}

void OffsetCurveSetBuilder::addLineString(const LineString *line){
	if (distance <= 0.0) return;
	CoordinateSequence *coord=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());
	vector<CoordinateSequence*> *lineList=curveBuilder->getLineCurve(coord, distance);
	delete coord;
	addCurves(lineList, Location::EXTERIOR, Location::INTERIOR);
	delete lineList;
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
	CoordinateSequence *shellCoord=CoordinateSequence::removeRepeatedPoints(shell->getCoordinatesRO());
	// optimization - don't bother computing buffer
	// if the polygon would be completely eroded
	if (distance < 0.0 && isErodedCompletely(shellCoord, distance))
	{
		delete shellCoord;
		return;
	}
	addPolygonRing(shellCoord,offsetDistance,offsetSide,Location::EXTERIOR,Location::INTERIOR);
	delete shellCoord;
	for (int i=0;i<p->getNumInteriorRing(); i++) {
		const LinearRing *hole=(const LinearRing *)p->getInteriorRingN(i);
		CoordinateSequence *holeCoord=CoordinateSequence::removeRepeatedPoints(hole->getCoordinatesRO());
		// optimization - don't bother computing buffer for this hole
		// if the hole would be completely covered
		if (distance > 0.0 && isErodedCompletely(holeCoord, -distance))
		{
//			delete holeCoord;
			continue;
		}
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CCW)
		addPolygonRing(holeCoord,offsetDistance,Position::opposite(offsetSide),Location::INTERIOR,Location::EXTERIOR);
		delete holeCoord;
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
void OffsetCurveSetBuilder::addPolygonRing(const CoordinateSequence *coord, double offsetDistance, int side, int cwLeftLoc, int cwRightLoc){
	//Coordinate[] coord=CoordinateArrays->removeRepeatedPoints(lr->getCoordinates());
	int leftLoc =cwLeftLoc;
	int rightLoc=cwRightLoc;
	if (cga->isCCW(coord)) {
		leftLoc=cwRightLoc;
		rightLoc=cwLeftLoc;
		side=Position::opposite(side);
	}
	vector<CoordinateSequence*> *lineList=curveBuilder->getRingCurve(coord, side, offsetDistance);
	addCurves(lineList, leftLoc, rightLoc);
	delete lineList;
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
bool OffsetCurveSetBuilder::isErodedCompletely(CoordinateSequence *ringCoord, double bufferDistance){
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
	LinearRing *ring=inputGeom->getFactory()->createLinearRing(*ringCoord);
	MinimumDiameter md(ring); //=new MinimumDiameter(ring);
	minDiam=md.getLength();
	delete ring;
	//delete md;
	//System->out->println(md->getDiameter());
	return minDiam < 2 * fabs(bufferDistance);
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
bool OffsetCurveSetBuilder::isTriangleErodedCompletely(CoordinateSequence *triangleCoord,double bufferDistance){
	Triangle *tri=new Triangle(triangleCoord->getAt(0), triangleCoord->getAt(1), triangleCoord->getAt(2));
	Coordinate *inCentre=tri->inCentre();
	double distToCentre=cga->distancePointLine(*inCentre, tri->p0, tri->p1);
	return distToCentre < fabs(bufferDistance);
}



}
