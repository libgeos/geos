#include "geosAlgorithm.h"
#include <typeinfo>
#include "graph.h"

PointLocator::PointLocator() {
	cga=new RobustCGAlgorithms();
}

/**
* locate determines the topological relationship (location) of a single point
* to a Geometry.
* It handles both single-element
* and multi-element Geometries.  The algorithm for multi-part Geometries
* is more complex, since it has to take into account the boundaryDetermination rule.
*
* @return the location of the point relative to the input Geometry
*/
int PointLocator::locate(Coordinate p,Geometry *geom) {
	if (geom->isEmpty()) return Location::EXTERIOR;
	if (typeid(*geom)==typeid(LineString)) {
		return locate(p,(LineString*) geom);
	}
	if (typeid(*geom)==typeid(LinearRing)) {
		return locate(p,(LinearRing*) geom);
	} else if (typeid(*geom)==typeid(Polygon)) {
		return locate(p,(Polygon*) geom);
	}

	isIn=false;
	numBoundaries=0;
	computeLocation(p,geom);
	if (GeometryGraph::isInBoundary(numBoundaries)) return Location::BOUNDARY;
	if (numBoundaries>0 || isIn) return Location::INTERIOR;
	return Location::EXTERIOR;
}

void PointLocator::computeLocation(Coordinate p,Geometry *geom) {
	if (typeid(*geom)==typeid(LineString)) {
		updateLocationInfo(locate(p,(LineString*) geom));
	}
	if (typeid(*geom)==typeid(LinearRing)) {
		updateLocationInfo(locate(p,(LinearRing*) geom));
	} else if (typeid(*geom)==typeid(Polygon)) {
		updateLocationInfo(locate(p,(Polygon*) geom));
	} else if (typeid(*geom)==typeid(MultiLineString)) {
		MultiLineString *ml=(MultiLineString*) geom;
		for(int i=0;i<ml->getNumGeometries();i++) {
			LineString *l=(LineString*) ml->getGeometryN(i);
			updateLocationInfo(locate(p,l));
		}
	} else if (typeid(*geom)==typeid(MultiPolygon)) {
		MultiPolygon *mpoly=(MultiPolygon*) geom;
		for(int i=0;i<mpoly->getNumGeometries();i++) {
			Polygon *poly=(Polygon*) mpoly->getGeometryN(i);
			updateLocationInfo(locate(p,poly));
		}
	} else if (typeid(*geom)==typeid(GeometryCollection)) {
		GeometryCollectionIterator geomi((GeometryCollection*) geom);
		while (geomi.hasNext()) {
			Geometry *g2=geomi.next();
//			if (! g2->equals(geom))
			if (g2!=geom)
				computeLocation(p,g2);
		}
	}
}

void PointLocator::updateLocationInfo(int loc) {
	if (loc==Location::INTERIOR) isIn=true;
	if (loc==Location::BOUNDARY) numBoundaries++;
}

int PointLocator::locate(Coordinate p,LineString *l) {
	CoordinateList pt(l->getCoordinates());
	if (! l->isClosed()) {
		if ((p==pt.getAt(0)) || (p==pt.getAt(pt.getSize()-1))) {
			return Location::BOUNDARY;
		}
	}
	if (cga->isOnLine(p,pt))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

int PointLocator::locate(Coordinate p,LinearRing *ring) {
	if (cga->isOnLine(p,ring->getCoordinates())) {
		return Location::BOUNDARY;
	}
	if (cga->isPointInRing(p,ring->getCoordinates()))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

int PointLocator::locate(Coordinate p,Polygon *poly) {
	if (poly->isEmpty()) return Location::EXTERIOR;

	LinearRing *shell=(LinearRing*) poly->getExteriorRing();

	int shellLoc=locate(p,shell);
	if (shellLoc==Location::EXTERIOR) return Location::EXTERIOR;
	if (shellLoc==Location::BOUNDARY) return Location::BOUNDARY;
	// now test if the point lies in or on the holes
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*) poly->getInteriorRingN(i);
		int holeLoc=locate(p,hole);
		if (holeLoc==Location::INTERIOR) return Location::EXTERIOR;
		if (holeLoc==Location::BOUNDARY) return Location::BOUNDARY;
	}
	return Location::INTERIOR;
}

