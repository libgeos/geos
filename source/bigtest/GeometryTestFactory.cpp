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

#include <math.h>

#include <geos/geom.h>
#include "bigtest.h"

#define PI 3.14159265358979

using namespace geos;

Polygon* GeometryTestFactory::createBox(GeometryFactory *fact,double minx,double miny,int nSide,double segLen) {
	CoordinateSequence *pts=createBox(minx, miny, nSide, segLen);
    return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

CoordinateSequence* GeometryTestFactory::createBox(double minx, double miny,int nSide,double segLen) {
	int i;
	CoordinateSequence *pts=new DefaultCoordinateSequence();
	double maxx=minx+nSide*segLen;
	double maxy=miny+nSide*segLen;

	for(i=0;i<nSide;i++) {
		double x=minx+i*segLen;
		double y=miny;
		pts->add(*(new Coordinate(x,y)));
	}
	for(i=0;i<nSide;i++) {
		double x=maxx;
		double y=miny+i*segLen;
		pts->add(*(new Coordinate(x,y)));
	}
	for(i=0;i<nSide;i++) {
		double x=maxx-i*segLen;
		double y=maxy;
		pts->add(*(new Coordinate(x,y)));
	}
	for(i=0;i<nSide;i++) {
		double x=minx;
		double y=maxy-i*segLen;
		pts->add(*(new Coordinate(x,y)));
	}
	pts->add(*(new Coordinate(pts->getAt(0))));
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
	CoordinateSequence *pts=new DefaultCoordinateSequence(nPts+1); 
	double len=size/2.0;

	for(int i=0;i<nPts;i++) {
		double ang=i*(2*PI/nPts);
		double x=len*cos(ang)+basex;
		double y=len*sin(ang)+basey;
		pts->add(*(new Coordinate(x,y)));
	}
	pts->add(*(new Coordinate(pts->getAt(0))));
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
	CoordinateSequence *pts=new DefaultCoordinateSequence(); 

	double starAng=0.0;

	for(int iArm=0;iArm<nArms;iArm++) {
		for(int iArmPt=0;iArmPt<nArmPt;iArmPt++) {
			double ang=iArmPt*(2*PI/nArmPt);
			double len=armLen*(1-cos(ang)/2)+armBaseLen;
			double x=len*cos(starAng+iArmPt*angInc/nArmPt)+basex;
			double y=len*sin(starAng+iArmPt*angInc/nArmPt)+basey;
			pts->add(*(new Coordinate(x,y)));
		}
		starAng+=angInc;
	}
	pts->add(*(new Coordinate(pts->getAt(0))));
	return pts;
}

Polygon* GeometryTestFactory::createSineStar(GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts){
	CoordinateSequence *pts=createSineStar(basex, basey, size, armLen, nArms, nPts);
	return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

/**********************************************************************
 * $Log$
 * Revision 1.12  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.11  2004/07/19 10:35:23  strk
 * bigtest.h moved to local dir
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
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

