#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

namespace geos {

InteriorPointLine::InteriorPointLine(Geometry *g) {
	interiorPoint=NULL;
	minDistance=DoubleInfinity;
	centroid=g->getCentroid()->getCoordinate();
	addInterior(g);
	if (interiorPoint==NULL)
		addEndpoints(g);
}

InteriorPointLine::~InteriorPointLine() {
}

/**
* Tests the interior vertices (if any)
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointLine::addInterior(Geometry *geom){
	if (typeid(*geom)==typeid(LineString)) {
		addInterior(geom->getCoordinates());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			addInterior(gc->getGeometryN(i));
		}
	}
}

void InteriorPointLine::addInterior(CoordinateList *pts) {
	for(int i=1;i<pts->getSize()-1;i++) {
		add(&(pts->getAt(i)));
	}
}

/**
* Tests the endpoint vertices
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointLine::addEndpoints(Geometry *geom) {
	if (typeid(*geom)==typeid(LineString)) {
		addEndpoints(geom->getCoordinates());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			addEndpoints(gc->getGeometryN(i));
		}
	}
}

void InteriorPointLine::addEndpoints(CoordinateList *pts){
	add(&(pts->getAt(0)));
	add(&(pts->getAt(pts->getSize()-1)));
}

void InteriorPointLine::add(Coordinate *point) {
	double dist=point->distance(*centroid);
	if (dist<minDistance) {
		interiorPoint=new Coordinate(*point);
		minDistance=dist;
	}
}

Coordinate* InteriorPointLine::getInteriorPoint() {
	return interiorPoint;
}
}