#include "../headers/geosAlgorithm.h"
#include <typeinfo>

namespace geos {

/**
* locate is the main location function.  It handles both single-element
* and multi-element Geometries.  The algorithm for multi-element Geometries
* is more complex, since it has to take into account the boundaryDetermination rule
*/
int SimplePointInAreaLocator::locate(Coordinate& p,Geometry *geom){
	if (geom->isEmpty()) return Location::EXTERIOR;
	if (containsPoint(p,geom))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

bool SimplePointInAreaLocator::containsPoint(Coordinate& p,Geometry *geom){
	if (typeid(*geom)==typeid(Polygon)) {
		return containsPointInPolygon(p,(Polygon*)geom);
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
			   (typeid(*geom)==typeid(MultiPoint)) ||
			   (typeid(*geom)==typeid(MultiLineString)) ||
			   (typeid(*geom)==typeid(MultiPolygon))) {
		GeometryCollectionIterator geomi((GeometryCollection*)geom);
		while (geomi.hasNext()) {
			Geometry *g2=geomi.next();
			if (g2!=geom)
				if (containsPoint(p,g2))
					return true;
		}
	}
	return false;
}

bool SimplePointInAreaLocator::containsPointInPolygon(Coordinate& p,Polygon *poly) {
	auto_ptr<CGAlgorithms> cga(new RobustCGAlgorithms());
	if (poly->isEmpty()) return false;
	LineString *shell=poly->getExteriorRing();
	if (!cga->isPointInRing(p,shell->getCoordinates())) {
		return false;
	}
	// now test if the point lies in or on the holes
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*)poly->getInteriorRingN(i);
		if (cga->isPointInRing(p,hole->getCoordinates())) {
			return false;
		}
	}
	return true;
}
}