#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

namespace geos {

double InteriorPointArea::avg(double a, double b){
	return (a+b)/2.0;
}

InteriorPointArea::InteriorPointArea(const Geometry *g) {
	interiorPoint=NULL;
	maxWidth=0.0;
	factory=new GeometryFactory(g->getPrecisionModel(),g->getSRID());
	add(g);
}

InteriorPointArea::~InteriorPointArea() {
	delete factory;
}

Coordinate* InteriorPointArea::getInteriorPoint() const {
	return interiorPoint;
}

/**
* Tests the interior vertices (if any)
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointArea::add(const Geometry *geom) {
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
void InteriorPointArea::addPolygon(const Geometry *geometry) {
	LineString *bisector=horizontalBisector(geometry);
	Geometry *intersections=bisector->intersection(geometry);
	const Geometry *widestIntersection=widestGeometry(intersections);
	Envelope *env=widestIntersection->getEnvelopeInternal();
	double width=env->getWidth();
	if (interiorPoint==NULL || width>maxWidth) {
		interiorPoint=centre(env);
		maxWidth = width;
	}
	delete env;
	delete bisector;
	delete intersections;
}

//@return if geometry is a collection, the widest sub-geometry; otherwise,
//the geometry itself
const Geometry* InteriorPointArea::widestGeometry(const Geometry *geometry) {
	if ((typeid(*geometry)==typeid(GeometryCollection)) ||
				(typeid(*geometry)==typeid(MultiPoint)) ||
				(typeid(*geometry)==typeid(MultiPolygon)) ||
				(typeid(*geometry)==typeid(MultiLineString))) {
		return widestGeometry((GeometryCollection*) geometry);
	} else {
		return geometry;
	}
}

const Geometry*
InteriorPointArea::widestGeometry(const GeometryCollection* gc) {
	if (gc->isEmpty()) {
		return gc;
	}
	const Geometry* widestGeometry=gc->getGeometryN(0);
	for(int i=1;i<gc->getNumGeometries();i++) { //Start at 1
		auto_ptr<Envelope> env1(gc->getGeometryN(i)->getEnvelopeInternal());
		auto_ptr<Envelope> env2(widestGeometry->getEnvelopeInternal());
		if (env1->getWidth()>env2->getWidth()) {
				widestGeometry=gc->getGeometryN(i);
		}
	}
	return widestGeometry;
}

LineString* InteriorPointArea::horizontalBisector(const Geometry *geometry) {
	Envelope *envelope=geometry->getEnvelopeInternal();
	// Assert: for areas, minx <> maxx
	double avgY=avg(envelope->getMinY(),envelope->getMaxY());
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	Coordinate *c1=new Coordinate(envelope->getMinX(),avgY);
	Coordinate *c2=new Coordinate(envelope->getMaxX(),avgY);
	cl->add(*c1);
	cl->add(*c2);
	delete c1;
	delete c2;
	delete envelope;
	return factory->createLineString(cl);
}

/**
* Returns the centre point of the envelope.
* @param envelope the envelope to analyze
* @return the centre of the envelope
*/
Coordinate* InteriorPointArea::centre(const Envelope *envelope) const {
	return new Coordinate(avg(envelope->getMinX(),
							  envelope->getMaxX()),
						  avg(envelope->getMinY(),
						      envelope->getMaxY()));
}
}

