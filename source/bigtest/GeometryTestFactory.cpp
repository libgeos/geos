#include <math.h>

#include "bigtest.h"
#include "../headers/geom.h"

#define PI 3.14159265358979

Polygon* GeometryTestFactory::createBox(GeometryFactory *fact,double minx,double miny,int nSide,double segLen) {
	CoordinateList *pts=createBox(minx, miny, nSide, segLen);
    return fact->createPolygon(fact->createLinearRing(pts),NULL);
}

CoordinateList* GeometryTestFactory::createBox(double minx, double miny,int nSide,double segLen) {
	int i;
	CoordinateList *pts=CoordinateListFactory::internalFactory->createCoordinateList();
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
	pts->add(Coordinate(pts->getAt(0)));
	return pts;
}

	/**
	* Creates a circle
	* @param x the centre x coord
	* @param y the centre y coord
	* @param size the size of the envelope of the star
	* @param nPts the number of points in the star
	*/
CoordinateList* GeometryTestFactory::createCircle(double basex,double basey,double size,int nPts) {
	CoordinateList *pts=CoordinateListFactory::internalFactory->createCoordinateList(nPts+1);
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
	CoordinateList *pts=createCircle(basex, basey, size, nPts);
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
CoordinateList* GeometryTestFactory::createSineStar(double basex,double basey,double size,double armLen,int nArms,int nPts) {
	double armBaseLen=size/2-armLen;
	if (armBaseLen<0) armBaseLen=0.5;

	double angInc=2*PI/nArms;
	int nArmPt=nPts/nArms;
	if (nArmPt<5) nArmPt=5;

	int nPts2=nArmPt*nArms;
	CoordinateList *pts=CoordinateListFactory::internalFactory->createCoordinateList();

	double starAng=0.0;

	for(int iArm=0;iArm<nArms;iArm++) {
		for(int iArmPt=0;iArmPt<nArmPt;iArmPt++) {
			double ang=iArmPt*(2*PI/nArmPt);
			double len=armLen*(1-cos(ang)/2)+armBaseLen;
			double x=len*cos(starAng+iArmPt*angInc/nArmPt)+basex;
			double y=len*sin(starAng+iArmPt*angInc/nArmPt)+basey;
			pts->add(Coordinate(x,y));
		}
		starAng+=angInc;
	}
	pts->add(Coordinate(pts->getAt(0)));
	return pts;
}

Polygon* GeometryTestFactory::createSineStar(GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts){
	CoordinateList *pts=createSineStar(basex, basey, size, armLen, nArms, nPts);
	return fact->createPolygon(fact->createLinearRing(pts),NULL);
}
