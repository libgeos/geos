/**********************************************************************
 * $Id: GeometryTestFactory.cpp 2163 2008-08-18 16:32:50Z mloskot $
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

#include <cmath>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Coordinate.h>
#include "bigtest.h"

#define PI 3.14159265358979

using namespace geos::geom;

Polygon* GeometryTestFactory::createBox(GeometryFactory *fact,double minx,double miny,int nSide,double segLen) {
	CoordinateSequence *pts=createBox(minx, miny, nSide, segLen);
    return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

CoordinateSequence* GeometryTestFactory::createBox(double minx, double miny,int nSide,double segLen) {
	int i;
	CoordinateSequence *pts=new CoordinateArraySequence();
	double maxx=minx+nSide*segLen;
	double maxy=miny+nSide*segLen;

	for(i=0;i<nSide;i++) {
		double x=minx+i*segLen;
		double y=miny;
		pts->add(Coordinate(x,y));
	}
	for(i=0;i<nSide;i++) {
		double x=maxx;
		double y=miny+i*segLen;
		pts->add(Coordinate(x,y));
	}
	for(i=0;i<nSide;i++) {
		double x=maxx-i*segLen;
		double y=maxy;
		pts->add(Coordinate(x,y));
	}
	for(i=0;i<nSide;i++) {
		double x=minx;
		double y=maxy-i*segLen;
		pts->add(Coordinate(x,y));
	}
	pts->add(pts->getAt(0));
	return pts;
}

	/**
	* Creates a circle
	* @param x the centre x coord
	* @param y the centre y coord
	* @param size the size of the envelope of the star
	* @param nPts the number of points in the star
	*/
CoordinateSequence* GeometryTestFactory::createCircle(double basex,double basey,double size,int nPts) {
	CoordinateSequence *pts=new CoordinateArraySequence(nPts+1); 
	double len=size/2.0;

	for(int i=0;i<nPts;i++) {
		double ang=i*(2*PI/nPts);
		double x=len*cos(ang)+basex;
		double y=len*sin(ang)+basey;
		pts->add(Coordinate(x,y));
	}
	pts->add(Coordinate(pts->getAt(0)));
	return pts;
}

Polygon* GeometryTestFactory::createCircle(GeometryFactory *fact,double basex,double basey,double size,int nPts) {
	CoordinateSequence *pts=createCircle(basex, basey, size, nPts);
    return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

	/**
	* Creates a star from a "circular" sine wave
	* @param basex the centre x coord
	* @param basey the centre y coord
	* @param size the size of the envelope of the star
	* @param armLen the length of an arm of the star
	* @param nArms the number of arms of the star
	* @param nPts the number of points in the star
	*/
CoordinateSequence* GeometryTestFactory::createSineStar(double basex,double basey,double size,double armLen,int nArms,int nPts) {
	double armBaseLen=size/2-armLen;
	if (armBaseLen<0) armBaseLen=0.5;

	double angInc=2*PI/nArms;
	int nArmPt=nPts/nArms;
	if (nArmPt<5) nArmPt=5;

	//int nPts2=nArmPt*nArms;
	CoordinateSequence *pts=new CoordinateArraySequence(); 

	double starAng=0.0;

	for(int iArm=0;iArm<nArms;iArm++) {
		for(int iArmPt=0;iArmPt<nArmPt;iArmPt++) {
			double ang=iArmPt*(2*PI/nArmPt);
			double len=armLen*(1-cos(ang)/2)+armBaseLen;
			double x=len*cos(starAng+iArmPt*angInc/nArmPt)+basex;
			double y=len*sin(starAng+iArmPt*angInc/nArmPt)+basey;

            // FIXME - mloskot: Number of problems here:
            // - new'd Coordinate definitely leaks
            // - add() method makes a copy
            // - why temporarily used Coordinate is allocated on the heap?!?
			pts->add(*(new Coordinate(x,y)));
		}
		starAng+=angInc;
	}
    // FIXME - mloskot: The same problems as above
	pts->add(*(new Coordinate(pts->getAt(0))));
	return pts;
}

Polygon* GeometryTestFactory::createSineStar(GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts){
	CoordinateSequence *pts=createSineStar(basex, basey, size, armLen, nArms, nPts);
	return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/01/31 19:07:35  strk
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
 * Revision 1.13  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.12  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.11  2004/07/19 10:35:23  strk
 * bigtest.h moved to local dir
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

