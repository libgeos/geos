#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"

CentroidArea::CentroidArea() {
	basePt=NULL;
	cga=new RobustCGAlgorithms();
	triangleCent3=new Coordinate();
	areasum2=0;
	cg3=new Coordinate();
}

CentroidArea::~CentroidArea() {
	delete cga;
	delete triangleCent3;
	delete cg3;
}

/**
* Adds the area defined by a Geometry to the centroid total.
* If the geometry has no area it does not contribute to the centroid.
*
* @param geom the geometry to add
*/
void CentroidArea::add(Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) {
		Polygon *poly=(Polygon*) geom;
		setBasePoint(&(poly->getCoordinates()->getAt(0)));
		add(poly);
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
* Adds the area defined by an array of
* coordinates.  The array must be a ring;
* i.e. end with the same coordinate as it starts with.
* @param ring an array of {@link Coordinate}s
*/
void CentroidArea::add(CoordinateList *ring) {
	setBasePoint(&(ring->getAt(0)));
	addShell(ring);
}

Coordinate& CentroidArea::getCentroid() {
	return *(new Coordinate(cg3->x/3/areasum2,cg3->y/3/areasum2));
}

void CentroidArea::setBasePoint(Coordinate *newBasePt) {
	if(basePt==NULL)
		basePt=newBasePt;
}

void CentroidArea::add(Polygon *poly) {
	addShell(poly->getExteriorRing()->getCoordinates());
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		addHole(poly->getInteriorRingN(i)->getCoordinates());
	}
}

void CentroidArea::addShell(CoordinateList *pts) {
	bool isPositiveArea=!cga->isCCW(pts);
	for(int i=0;i<pts->getSize()-1;i++) {
		addTriangle(*basePt,pts->getAt(i),pts->getAt(i+1),isPositiveArea);
	}
}

void CentroidArea::addHole(CoordinateList *pts){
	bool isPositiveArea=cga->isCCW(pts);
	for(int i=0;i<pts->getSize()-1;i++) {
		addTriangle(*basePt,pts->getAt(i),pts->getAt(i+1),isPositiveArea);
	}
}

void CentroidArea::addTriangle(Coordinate &p0,Coordinate &p1,Coordinate &p2,bool isPositiveArea){
	double sign=(isPositiveArea)?1.0:-1.0;
	centroid3(p0,p1,p2,triangleCent3);
	double area2res=area2(p0,p1,p2);
	cg3->x+=sign*area2res*triangleCent3->x;
	cg3->y+=sign*area2res*triangleCent3->y;
	areasum2+=sign*area2res;
}
/**
* Returns three times the centroid of the triangle p1-p2-p3.
* The factor of 3 is
* left in to permit division to be avoided until later.
*/
void CentroidArea::centroid3(Coordinate &p1,Coordinate &p2,Coordinate &p3,Coordinate *c){
	c->x=p1.x+p2.x+p3.x;
	c->y=p1.y+p2.y+p3.y;
}

/**
* Returns twice the signed area of the triangle p1-p2-p3,
* positive if a,b,c are oriented ccw, and negative if cw.
*/
double CentroidArea::area2(Coordinate &p1,Coordinate &p2,Coordinate &p3){
	return (p2.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(p2.y-p1.y);
}
