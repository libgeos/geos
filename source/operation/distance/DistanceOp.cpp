/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.6  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/


#include "../../headers/opDistance.h"

namespace geos {

double DistanceOp::distance(const Geometry *g0, const Geometry *g1) {
	auto_ptr<DistanceOp> distOp(new DistanceOp(g0,g1));
	return distOp->distance();
}

DistanceOp::DistanceOp(const Geometry *g0, const Geometry *g1){
	ptLocator=new PointLocator();
	minDistance=DoubleInfinity;
	geom=new vector<const Geometry*>(2);
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
	vector<const Geometry*> *polys0=PolygonExtracterFilter::getPolygons((*geom)[0]);
	vector<const Geometry*> *polys1=PolygonExtracterFilter::getPolygons((*geom)[1]);
	// test if either geometry is wholely inside the other
	if (polys1->size()>0) {
		vector<const Coordinate*> *insidePts0=ConnectedElementPointFilter::getCoordinates((*geom)[0]);
		computeInside(insidePts0,polys1);
		delete insidePts0;
		if (minDistance<=0.0) {
			delete polys0;
			delete polys1;
			return;
		}
	}
	if (polys0->size()>0) {
		vector<const Coordinate*> *insidePts1=ConnectedElementPointFilter::getCoordinates((*geom)[1]);
		computeInside(insidePts1,polys0);
		delete insidePts1;
		if (minDistance<=0.0) {
			delete polys0;
			delete polys1;
			return;
		}
	}
	vector<const Geometry*> *lines0=LineExtracterFilter::getLines((*geom)[0]);
	vector<const Geometry*> *lines1=LineExtracterFilter::getLines((*geom)[1]);
	vector<const Geometry*> *pts0=PointExtracterFilter::getPoints((*geom)[0]);
	vector<const Geometry*> *pts1=PointExtracterFilter::getPoints((*geom)[1]);
	computeMinDistanceLines(lines0,lines1);
	if (minDistance<=0.0) {
		delete polys0;
		delete polys1;
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	}
	computeMinDistanceLinesPoints(lines0,pts1);
	if (minDistance<=0.0) {
		delete polys0;
		delete polys1;
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	}
	computeMinDistanceLinesPoints(lines1,pts0);
	if (minDistance<=0.0) {
		delete polys0;
		delete polys1;
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	}
	computeMinDistancePoints(pts0,pts1);
	delete polys0;
	delete polys1;
	delete lines0;
	delete lines1;
	delete pts0;
	delete pts1;
}

void DistanceOp::computeInside(vector<const Coordinate*> *pts,vector<const Geometry*> *polys){
	for (int i=0;i<(int)pts->size();i++) {
		const Coordinate *pt=(*pts)[i];
		for (int j=0;j<(int)polys->size();j++) {
			const Polygon *poly=(Polygon*) (*polys)[j];
			computeInside(pt,poly);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeInside(const Coordinate *pt,const Polygon *poly){
	if (Location::EXTERIOR!=ptLocator->locate(*pt,poly))
		minDistance=0.0;
}

void DistanceOp::computeMinDistanceLines(vector<const Geometry*> *lines0,vector<const Geometry*> *lines1){
	for (int i=0;i<(int)lines0->size();i++) {
		const LineString *line0=(LineString*) (*lines0)[i];
		for (int j=0;j<(int)lines1->size();j++) {
			const LineString *line1=(LineString*) (*lines1)[j];
			computeMinDistance(line0,line1);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistancePoints(vector<const Geometry*> *points0,vector<const Geometry*> *points1){
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

void DistanceOp::computeMinDistanceLinesPoints(vector<const Geometry*> *lines,vector<const Geometry*> *points){
	for (int i=0;i<(int)lines->size();i++) {
		LineString *line=(LineString*) (*lines)[i];
		for (int j=0;j<(int)points->size();j++) {
			Point *pt=(Point*) (*points)[j];
			computeMinDistance(line,pt);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeMinDistance(const LineString *line0, const LineString *line1) {
	Envelope *env0=line0->getEnvelopeInternal();
	Envelope *env1=line1->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		delete env0;
		delete env1;
		return;
	}
	delete env0;
	delete env1;
	const CoordinateList *coord0=line0->getCoordinatesRO();
	const CoordinateList *coord1=line1->getCoordinatesRO();
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

void DistanceOp::computeMinDistance(const LineString *line, const Point *pt){
	Envelope *env0=line->getEnvelopeInternal();
	Envelope *env1=pt->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		delete env0;
		delete env1;
		return;
	}
	delete env0;
	delete env1;
	const CoordinateList *coord0=line->getCoordinatesRO();
	const Coordinate *coord=pt->getCoordinate();
	// brute force approach!
	for(int i=0;i<coord0->getSize()-1;i++) {
		double dist=CGAlgorithms::distancePointLine(*coord,coord0->getAt(i),coord0->getAt(i+1));
		updateMinDistance(dist);
		if (minDistance<=0.0) return;
	}
}
}

