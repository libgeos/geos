#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"

/**
* Computes the (approximate) intersection point between two line segments
* using homogeneous coordinates.
* <p>
* Note that this algorithm is
* not numerically stable; i.e. it can produce intersection points which
* lie outside the envelope of the line segments themselves.  In order
* to increase the precision of the calculation input points should be normalized
* before passing them to this routine.
*/
Coordinate& HCoordinate::intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2) {
	HCoordinate *intHCoord=new HCoordinate(HCoordinate(HCoordinate(p1),HCoordinate(p2)),HCoordinate(HCoordinate(q1),HCoordinate(q2)));
	return intHCoord->getCoordinate();
}

HCoordinate::HCoordinate(){
	x = 0.0;
	y = 0.0;
	w = 1.0;
}

HCoordinate::HCoordinate(double _x, double _y, double _w) {
	x = _x;
	y = _y;
	w = _w;
}

HCoordinate::HCoordinate(Coordinate& p) {
	x = p.x;
	y = p.y;
	w = 1.0;
}

HCoordinate::HCoordinate(HCoordinate p1, HCoordinate p2) {
	x = p1.y*p2.w - p2.y*p1.w;
	y = p2.x*p1.w - p1.x*p2.w;
	w = p1.x*p2.y - p2.x*p1.y;
}

double HCoordinate::getX() {
	double a = x/w;
	if ((a==DoubleNotANumber)||(a==DoubleInfinity)||(a==DoubleNegInfinity)) {
		throw new NotRepresentableException();
	}
	return a;
}

double HCoordinate::getY() {
	double a = y/w;
	if ((a==DoubleNotANumber)||(a==DoubleInfinity)||(a==DoubleNegInfinity)) {
		throw new NotRepresentableException();
	}
	return a;
}

Coordinate& HCoordinate::getCoordinate() {
	return *(new Coordinate(getX(),getY()));
}
