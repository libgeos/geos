#ifndef GEOS_BIGTEST_H
#define GEOS_BIGTEST_H

#include "geom.h"

using namespace std;

class GeometryTestFactory {
public:
	static Polygon* createBox(GeometryFactory *fact,double minx,double miny,int nSide,double segLen);
	static CoordinateList* createBox(double minx,double miny,int nSide,double segLen);
	static CoordinateList* createCircle(double basex,double basey,double size,int nPts);
	static Polygon* createCircle(GeometryFactory *fact,double basex,double basey,double size,int nPts);
	static CoordinateList* createSineStar(double basex,double basey,double size,double armLen,int nArms,int nPts);
	static Polygon* createSineStar(GeometryFactory *fact,double basex,double basey,double size,double armLen,int nArms,int nPts);
};

#endif
