#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

namespace geos {

double InteriorPointArea::avg(double a, double b){
	return (a+b)/2.0;
}

InteriorPointArea::InteriorPointArea(Geometry *g) {
	interiorPoint=NULL;
	maxWidth=0.0;
	factory=new GeometryFactory(g->getPrecisionModel(),g->getSRID());
	add(g);
}

InteriorPointArea::~InteriorPointArea() {
	delete factory;
}

Coordinate* InteriorPointArea::getInteriorPoint(){
	return interiorPoint;
}

/**
* Tests the interior vertices (if any)
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointArea::add(Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) {
		addPolygon(geom);
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
* Finds a reasonable point at which to label a Geometry.
* @param geometry the geometry to analyze
* @return the midpoint of the largest intersection between the geometry and
* a line halfway down its envelope
*/
void InteriorPointArea::addPolygon(Geometry *geometry) {
	LineString *bisector=horizontalBisector(geometry);
	Geometry *intersections=bisector->intersection(geometry);
	Geometry *widestIntersection=widestGeometry(intersections);
	double width=widestIntersection->getEnvelopeInternal()->getWidth();
	if (interiorPoint==NULL || width>maxWidth) {
		interiorPoint=centre(widestIntersection->getEnvelopeInternal());
		maxWidth = width;
	}
}

//@return if geometry is a collection, the widest sub-geometry; otherwise,
//the geometry itself
Geometry* InteriorPointArea::widestGeometry(Geometry *geometry) {
	if ((typeid(*geometry)==typeid(GeometryCollection)) ||
				(typeid(*geometry)==typeid(MultiPoint)) ||
				(typeid(*geometry)==typeid(MultiPolygon)) ||
				(typeid(*geometry)==typeid(MultiLineString))) {
		return widestGeometry((GeometryCollection*) geometry);
	} else {
		return geometry;
	}
}

Geometry* InteriorPointArea::widestGeometry(GeometryCollection* gc) {
	if (gc->isEmpty()) {
		return gc;
	}
	Geometry* widestGeometry=gc->getGeometryN(0);
	for(int i=1;i<gc->getNumGeometries();i++) { //Start at 1
		if (gc->getGeometryN(i)->getEnvelopeInternal()->getWidth() >
			widestGeometry->getEnvelopeInternal()->getWidth()) {
				widestGeometry=gc->getGeometryN(i);
		}
	}
	return widestGeometry;
}

LineString* InteriorPointArea::horizontalBisector(Geometry *geometry) {
	Envelope *envelope=geometry->getEnvelopeInternal();
	// Assert: for areas, minx <> maxx
	double avgY=avg(envelope->getMinY(),envelope->getMaxY());
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	cl->add(*(new Coordinate(envelope->getMinX(),avgY)));
	cl->add(*(new Coordinate(envelope->getMaxX(),avgY)));
	return factory->createLineString(cl);
}

/**
* Returns the centre point of the envelope.
* @param envelope the envelope to analyze
* @return the centre of the envelope
*/
Coordinate* InteriorPointArea::centre(Envelope *envelope) {
	return new Coordinate(avg(envelope->getMinX(),
							  envelope->getMaxX()),
						  avg(envelope->getMinY(),
						      envelope->getMaxY()));
}
}

