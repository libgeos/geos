#include "geosAlgorithm.h"
#include <typeinfo>

CGAlgorithms* SimplePointInAreaLocator::cga=new RobustCGAlgorithms();

/**
* locate is the main location function.  It handles both single-element
* and multi-element Geometries.  The algorithm for multi-element Geometries
* is more complex, since it has to take into account the boundaryDetermination rule
*/
int SimplePointInAreaLocator::locate(Coordinate p,Geometry *geom){
	if (geom->isEmpty()) return Location::EXTERIOR;
	if (containsPoint(p,geom))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

bool SimplePointInAreaLocator::containsPoint(Coordinate p,Geometry *geom){
	if (typeid(*geom)==typeid(Polygon)) {
		return containsPointInPolygon(p,(Polygon*)geom);
	} else if (typeid(*geom)==typeid(GeometryCollection)) {
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

bool SimplePointInAreaLocator::containsPointInPolygon(Coordinate p,Polygon *poly) {
	if (poly->isEmpty()) return false;
	LineString *shell=poly->getExteriorRing();
	if (!cga->isPointInPolygon(p,shell->getCoordinates()))
		return false;
	// now test if the point lies in or on the holes
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*)poly->getInteriorRingN(i);
		if (cga->isPointInPolygon(p,hole->getCoordinates()))
			return false;
	}
	return true;
}
