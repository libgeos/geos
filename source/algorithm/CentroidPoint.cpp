#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

namespace geos {

CentroidPoint::CentroidPoint() {
	ptCount=0;
	centSum=new Coordinate();
}

CentroidPoint::~CentroidPoint() {
	delete centSum;
}

/**
* Adds the point(s) defined by a Geometry to the centroid total.
* If the geometry is not of dimension 0 it does not contribute to the centroid.
* @param geom the geometry to add
*/
void CentroidPoint::add(Geometry *geom) {
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

/**
* Adds the length defined by an array of coordinates.
* @param pts an array of {@link Coordinate}s
*/
void CentroidPoint::add(Coordinate *pt) {
	ptCount+=1;
	centSum->x+=pt->x;
	centSum->y+=pt->y;
}

Coordinate* CentroidPoint::getCentroid() {
	return new Coordinate(centSum->x/ptCount,centSum->y/ptCount);
}
}

