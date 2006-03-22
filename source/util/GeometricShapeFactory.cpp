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
 **********************************************************************/

#include <geos/util/GeometricShapeFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>

#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace util { // geos.util

/*
 * Create a shape factory which will create shapes using the given
 * GeometryFactory. The given GeometryFactory will need to stay
 * alive for the whole instance lifetime (the object will keep
 * an external reference to it)
 */
GeometricShapeFactory::GeometricShapeFactory(const GeometryFactory* factory){
	//dim=new Dimensions();
	nPts=100;
	geomFact=factory;
}

GeometricShapeFactory::~GeometricShapeFactory() {
	//delete dim;
}


/**
* Sets the location of the shape by specifying the base coordinate
* (which in most cases is the
* lower left point of the envelope containing the shape).
*
* @param base the base coordinate of the shape
*/
void GeometricShapeFactory::setBase(const Coordinate& base)  {
	dim.setBase(base);
}
/**
* Sets the location of the shape by specifying the centre of
* the shape's bounding box
*
* @param centre the centre coordinate of the shape
*/
void GeometricShapeFactory::setCentre(const Coordinate& centre)  {
	dim.setCentre(centre);
}

/**
* Sets the total number of points in the created Geometry
*/
void GeometricShapeFactory::setNumPoints(int nNPts) {
	nPts=nNPts;
}

/**
* Sets the size of the extent of the shape in both x and y directions.
*
* @param size the size of the shape's extent
*/
void GeometricShapeFactory::setSize(double size) { 
	dim.setSize(size);
}

/**
* Sets the width of the shape.
*
* @param width the width of the shape
*/
void GeometricShapeFactory::setWidth(double width) {
	dim.setWidth(width);
}

/**
* Sets the height of the shape.
*
* @param height the height of the shape
*/
void GeometricShapeFactory::setHeight(double height) { 
	dim.setHeight(height);
}

/*
 * Creates a rectangular Polygon.
 */
Polygon* GeometricShapeFactory::createRectangle(){
	int i;
	int ipt = 0;
	int nSide = nPts / 4;
	if (nSide < 1) nSide = 1;
	Envelope *env = dim.getEnvelope();
	double XsegLen = env->getWidth() / nSide;
	double YsegLen = env->getHeight() / nSide;

	vector<Coordinate> *vc = new vector<Coordinate>(4*nSide+1);
	//CoordinateSequence* pts=new CoordinateArraySequence(4*nSide+1);

	//double maxx = env->getMinX() + nSide * XsegLen;
	//double maxy = env->getMinY() + nSide * XsegLen;

	Coordinate c;
	for (i = 0; i < nSide; i++) {
		double x = env->getMinX() + i * XsegLen;
		double y = env->getMinY();
		//pts->setAt(Coordinate(x, y),ipt++);
		c.x = x; c.y = y;
		(*vc)[ipt++] = c;
	}
	for (i = 0; i < nSide; i++) {
		double x = env->getMaxX();
		double y = env->getMinY() + i * YsegLen;
		//pts->setAt(*(new Coordinate(x, y)),ipt++);
		//pts->setAt(Coordinate(x, y),ipt++);
		c.x = x; c.y = y;
		(*vc)[ipt++] = c;
	}
	for (i = 0; i < nSide; i++) {
		double x = env->getMaxX() - i * XsegLen;
		double y = env->getMaxY();
		//pts->setAt(*(new Coordinate(x, y)),ipt++);
		//pts->setAt(Coordinate(x, y),ipt++);
		c.x = x; c.y = y;
		(*vc)[ipt++] = c;
	}
	for (i = 0; i < nSide; i++) {
		double x = env->getMinX();
		double y = env->getMaxY() - i * YsegLen;
		//pts->setAt(*(new Coordinate(x, y)),ipt++);
		//pts->setAt(Coordinate(x, y),ipt++);
		c.x = x; c.y = y;
		(*vc)[ipt++] = c;
	}
	delete env;
	//pts->setAt(*(new Coordinate(pts->getAt(0))),ipt++);
	//pts->setAt(Coordinate(pts->getAt(0)),ipt++);
	(*vc)[ipt++] = (*vc)[0];
	CoordinateSequence *cs = geomFact->getCoordinateSequenceFactory()->create(vc);
	LinearRing* ring=geomFact->createLinearRing(cs);
	Polygon* poly=geomFact->createPolygon(ring, NULL);
	return poly;
}

/*
 * Creates a circular Polygon.
 */
Polygon* GeometricShapeFactory::createCircle() {
	Envelope* env = dim.getEnvelope();
	double xRadius = env->getWidth() / 2.0;
	double yRadius = env->getHeight() / 2.0;

	double centreX = env->getMinX() + xRadius;
	double centreY = env->getMinY() + yRadius;
	delete env;

	vector<Coordinate>*pts=new vector<Coordinate>(nPts+1);
	int iPt = 0;
	Coordinate pt;
	for (int i = 0; i < nPts; i++) {
		double ang = i * (2 * 3.14159265358979 / nPts);
		double x = xRadius * cos(ang) + centreX;
		double y = yRadius * sin(ang) + centreY;
		pt.x = x;
		pt.y = y;
		(*pts)[iPt++] = pt;
	}
	(*pts)[iPt++] = (*pts)[0];
	CoordinateSequence *cs=geomFact->getCoordinateSequenceFactory()->create(pts);
	LinearRing* ring = geomFact->createLinearRing(cs);
	Polygon* poly=geomFact->createPolygon(ring,NULL);
	return poly;
}

/**
* Creates a elliptical arc, as a LineString.
*
* @return an elliptical arc
*/
LineString* GeometricShapeFactory::createArc(double startAng,double endAng){
	Envelope* env = dim.getEnvelope();
	double xRadius = env->getWidth() / 2.0;
	double yRadius = env->getHeight() / 2.0;

	double centreX = env->getMinX() + xRadius;
	double centreY = env->getMinY() + yRadius;
	delete env;

	double angSize = (endAng - startAng);
	if (angSize <= 0.0 || angSize > 2 * M_PI) //3.14159265358979
		angSize = 2 * M_PI; //3.14159265358979;
	double angInc = angSize / nPts;

	vector<Coordinate> *pts = new vector<Coordinate>(nPts);
	int iPt = 0;
	Coordinate pt;
	for (int i = 0; i < nPts; i++) {
		double ang = startAng + i * angInc;
		double x = xRadius * cos(ang) + centreX;
		double y = yRadius * sin(ang) + centreY;
		pt.x = x;
		pt.y = y;
		geomFact->getPrecisionModel()->makePrecise(&pt);
		(*pts)[iPt++] = pt;
	}
	CoordinateSequence *cs = geomFact->getCoordinateSequenceFactory()->create(pts);
	LineString* line = geomFact->createLineString(cs);
	return line;
}

GeometricShapeFactory::Dimensions::Dimensions()
	:
	base(Coordinate::getNull()),
	centre(Coordinate::getNull())
{
}

void GeometricShapeFactory::Dimensions::setBase(const Coordinate& newBase)  {
	base=newBase;
}

void GeometricShapeFactory::Dimensions::setCentre(const Coordinate& newCentre)  {
	centre=newCentre;
}

void GeometricShapeFactory::Dimensions::setSize(double size){
	height = size;
	width = size;
}

void GeometricShapeFactory::Dimensions::setWidth(double nWidth) {
	width=nWidth;
}
void GeometricShapeFactory::Dimensions::setHeight(double nHeight) {
	height=nHeight;
}

Envelope* GeometricShapeFactory::Dimensions::getEnvelope() {
	if (base!=Coordinate::getNull()) {
		return new Envelope(base.x, base.x + width, base.y, base.y + height);
	}
	if (centre!=Coordinate::getNull()) {
		return new Envelope(centre.x - width/2, centre.x + width/2,centre.y - height/2, centre.y + height/2);
	}
	return new Envelope(0, width, 0, height);
}

} // namespace geos.util
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/22 16:58:35  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 **********************************************************************/

