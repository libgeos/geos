#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

InteriorPointPoint::InteriorPointPoint(Geometry *g) {
	interiorPoint=NULL;
	minDistance=DoubleInfinity;
	centroid=g->getCentroid()->getCoordinate();
	add(g);
}

InteriorPointPoint::~InteriorPointPoint() {
}

/**
* Tests the point(s) defined by a Geometry for the best inside point.
* If a Geometry is not of dimension 0 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointPoint::add(Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) {
		add(geom->getCoordinate());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

void InteriorPointPoint::add(Coordinate *point) {
	double dist=point->distance(*centroid);
	if (dist<minDistance) {
		interiorPoint=new Coordinate(*point);
		minDistance=dist;
	}
}

Coordinate* InteriorPointPoint::getInteriorPoint() {
	return interiorPoint;
}
