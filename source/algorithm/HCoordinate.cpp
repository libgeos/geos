#include "geosAlgorithm.h"
#include "platform.h"

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

HCoordinate::HCoordinate(Coordinate p) {
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
		throw NotRepresentableException();
	}
	return a;
}

double HCoordinate::getY() {
	double a = y/w;
	if ((a==DoubleNotANumber)||(a==DoubleInfinity)||(a==DoubleNegInfinity)) {
		throw NotRepresentableException();
	}
	return a;
}

Coordinate HCoordinate::getCoordinate() {
	return Coordinate(getX(),getY());
}
