#include "../../headers/opDistance.h"

namespace geos {

double DistanceOp::distance(Geometry *g0,Geometry *g1) {
	DistanceOp *distOp=new DistanceOp(g0,g1);
	return distOp->distance();
}

DistanceOp::DistanceOp(Geometry *g0,Geometry *g1){
	ptLocator=new PointLocator();
	minDistance=DoubleInfinity;
	geom=new vector<Geometry*>;
	(*geom)[0]=g0;
	(*geom)[1]=g1;
}

DistanceOp::~DistanceOp(){
	delete ptLocator;
	delete geom;
}

double DistanceOp::distance() {
	computeMinDistance();
	return minDistance;
}

void DistanceOp::updateMinDistance(double dist) {
	if (dist<minDistance)
	minDistance=dist;
}

void DistanceOp::computeMinDistance() {
	vector<Geometry*> *polys0=PolygonExtracterFilter::getPolygons((*geom)[0]);
	vector<Geometry*> *polys1=PolygonExtracterFilter::getPolygons((*geom)[1]);
	// test if either geometry is wholely inside the other
	if (polys1->size()>0) {
		vector<Coordinate*> *insidePts0=ConnectedElementPointFilter::getCoordinates((*geom)[0]);
		computeInside(insidePts0,polys1);
		if (minDistance<=0.0) return;
	}
	if (polys0->size()>0) {
		vector<Coordinate*> *insidePts1=ConnectedElementPointFilter::getCoordinates((*geom)[1]);
		computeInside(insidePts1,polys0);
		if (minDistance<=0.0) return;
	}
	vector<Geometry*> *lines0=LineExtracterFilter::getLines((*geom)[0]);
	vector<Geometry*> *lines1=LineExtracterFilter::getLines((*geom)[1]);
	vector<Geometry*> *pts0=PointExtracterFilter::getPoints((*geom)[0]);
	vector<Geometry*> *pts1=PointExtracterFilter::getPoints((*geom)[1]);
	computeMinDistanceLines(lines0,lines1);
	if (minDistance<=0.0) return;
	computeMinDistanceLinesPoints(lines0,pts1);
	if (minDistance<=0.0) return;
	computeMinDistanceLinesPoints(lines1,pts0);
	if (minDistance<=0.0) return;
	computeMinDistancePoints(pts0,pts1);
}

void DistanceOp::computeInside(vector<Coordinate*> *pts,vector<Geometry*> *polys){
	for (int i=0;i<(int)pts->size();i++) {
		Coordinate *pt=(*pts)[i];
		for (int j=0;j<(int)polys->size();j++) {
			Polygon *poly=(Polygon*) (*polys)[j];
			computeInside(pt,poly);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeInside(Coordinate *pt,Polygon *poly){
	if (Location::EXTERIOR!=ptLocator->locate(*pt,poly))
		minDistance=0.0;
}

void DistanceOp::computeMinDistanceLines(vector<Geometry*> *lines0,vector<Geometry*> *lines1){
	for (int i=0;i<(int)lines0->size();i++) {
		LineString *line0=(LineString*) (*lines0)[i];
		for (int j=0;j<(int)lines1->size();j++) {
			LineString *line1=(LineString*) (*lines1)[j];
			computeMinDistance(line0,line1);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistancePoints(vector<Geometry*> *points0,vector<Geometry*> *points1){
	for (int i=0;i<(int)points0->size();i++) {
		Point *pt0=(Point*) (*points0)[i];
		for (int j=0;j<(int)points1->size();j++) {
			Point *pt1=(Point*) (*points1)[j];
			double dist=pt0->getCoordinate()->distance(*(pt1->getCoordinate()));
			updateMinDistance(dist);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistanceLinesPoints(vector<Geometry*> *lines,vector<Geometry*> *points){
	for (int i=0;i<(int)lines->size();i++) {
		LineString *line=(LineString*) (*lines)[i];
		for (int j=0;j<(int)points->size();j++) {
			Point *pt=(Point*) (*points)[j];
			computeMinDistance(line,pt);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistance(LineString *line0,LineString *line1) {
	if (line0->getEnvelopeInternal()->distance(line1->getEnvelopeInternal())>minDistance)
		return;
	CoordinateList *coord0=line0->getCoordinates();
	CoordinateList *coord1=line1->getCoordinates();
	// brute force approach!
	for(int i=0;i<coord0->getSize()-1;i++) {
		for(int j=0;j<coord1->getSize()-1;j++) {
			double dist=CGAlgorithms::distanceLineLine(coord0->getAt(i),coord0->getAt(i+1),
				coord1->getAt(j),coord1->getAt(j+1));
			updateMinDistance(dist);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistance(LineString *line,Point *pt){
	if (line->getEnvelopeInternal()->distance(pt->getEnvelopeInternal())>minDistance)
		return;
	CoordinateList *coord0=line->getCoordinates();
	Coordinate *coord=pt->getCoordinate();
	// brute force approach!
	for(int i=0;i<coord0->getSize()-1;i++) {
		double dist=CGAlgorithms::distancePointLine(*coord,coord0->getAt(i),coord0->getAt(i+1));
		updateMinDistance(dist);
		if (minDistance<=0.0) return;
	}
}
}