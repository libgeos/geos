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
 * Revision 1.15  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.14  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.13  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.12  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.10  2004/04/14 10:56:38  strk
 * Uncommented initializzazion and destruction of DistanceOp::minDistanceLocation
 *
 * Revision 1.9  2004/04/13 10:05:51  strk
 * GeometryLocation constructor made const-correct.
 * Fixed erroneus down-casting in DistanceOp::computeMinDistancePoints.
 *
 * Revision 1.8  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.6  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/


#include <geos/opDistance.h>
#include <geos/geomUtil.h>

namespace geos {

/**
* Compute the distance between the closest points of two geometries.
* @param g0 a {@link Geometry}
* @param g1 another {@link Geometry}
* @return the distance between the geometries
*/
double DistanceOp::distance(const Geometry *g0, const Geometry *g1) {
	DistanceOp distOp(g0,g1);
	return distOp.distance();
}

/**
* Compute the the closest points of two geometries.
* The points are presented in the same order as the input Geometries.
*
* @param g0 a {@link Geometry}
* @param g1 another {@link Geometry}
* @return the closest points in the geometries
*/
CoordinateSequence* DistanceOp::closestPoints(Geometry *g0,Geometry *g1){
	DistanceOp distOp(g0,g1);
	return distOp.closestPoints();
}

DistanceOp::DistanceOp(const Geometry *g0, const Geometry *g1):
	geom(2)
{
	geom[0]=g0;
	geom[1]=g1;
	minDistance=DoubleInfinity;
	minDistanceLocation=NULL;
}

DistanceOp::~DistanceOp(){
	unsigned int i;
	for (i=0; i<newCoords.size(); i++) delete newCoords[i];
	if ( minDistanceLocation )
	{
		for (i=0; i<minDistanceLocation->size(); i++)
		{
			delete (*minDistanceLocation)[i];
		}
		delete minDistanceLocation;
	}
}

/**
* Report the distance between the closest points on the input geometries.
*
* @return the distance between the geometries
*/
double DistanceOp::distance() {
	computeMinDistance();
	return minDistance;
}


/**
* Report the coordinates of the closest points in the input geometries.
* The points are presented in the same order as the input Geometries.
*
* @return a pair of Coordinate s of the closest points
*/
CoordinateSequence* DistanceOp::closestPoints() {
	computeMinDistance();
	CoordinateSequence* closestPts=new DefaultCoordinateSequence();
	closestPts->add((*minDistanceLocation)[0]->getCoordinate());
	closestPts->add((*minDistanceLocation)[1]->getCoordinate());
	return closestPts;
}

/**
* Report the locations of the closest points in the input geometries.
* The locations are presented in the same order as the input Geometries.
*
* @return a pair of {@link GeometryLocation}s for the closest points
*/
vector<GeometryLocation*>* DistanceOp::closestLocations(){
	computeMinDistance();
	return minDistanceLocation;
}

void DistanceOp::updateMinDistance(double dist) {
	if (dist<minDistance)
	minDistance=dist;
}

void DistanceOp::updateMinDistance(vector<GeometryLocation*> *locGeom, bool flip){
	// if not set then don't update
	if ((*locGeom)[0]==NULL) return;
	delete (*minDistanceLocation)[0];
	delete (*minDistanceLocation)[1];
	if (flip) {
		(*minDistanceLocation)[0]=(*locGeom)[1];
		(*minDistanceLocation)[1]=(*locGeom)[0];
	} else {
		(*minDistanceLocation)[0]=(*locGeom)[0];
		(*minDistanceLocation)[1]=(*locGeom)[1];
	}
}

void DistanceOp::computeMinDistance() {
    if (minDistanceLocation!=NULL) return;
    minDistanceLocation = new vector<GeometryLocation*>(2);
    computeContainmentDistance();
    if (minDistance<=0.0) return;
    computeLineDistance();
}

void DistanceOp::computeContainmentDistance() {
	vector<Geometry*> *polys0 = PolygonExtracter::getPolygons(geom[0]);
	vector<Geometry*> *polys1 = PolygonExtracter::getPolygons(geom[1]);
	vector<GeometryLocation*> *locPtPoly = new vector<GeometryLocation*>(2);
	// test if either geometry is wholely inside the other
	if (polys1->size()>0) {
		vector<GeometryLocation*> *insideLocs0 = ConnectedElementLocationFilter::getLocations(geom[0]);
		computeInside(insideLocs0, polys1, locPtPoly);
		if (minDistance <= 0.0) {
			(*minDistanceLocation)[0] = (*locPtPoly)[0];
			(*minDistanceLocation)[1] = (*locPtPoly)[1];
			delete polys0;
			delete polys1;
			delete locPtPoly;
			for (unsigned int i=0; i<insideLocs0->size(); i++)
			{
				GeometryLocation *l = (*insideLocs0)[i];
				if ( l != (*minDistanceLocation)[0] &&
					l != (*minDistanceLocation)[1] )
				{
					delete l;
				}
			}
			delete insideLocs0;
			return;
		}
		for (unsigned int i=0; i<insideLocs0->size(); i++)
			delete (*insideLocs0)[i];
		delete insideLocs0;
	}
	if (polys0->size()>0) {
		vector<GeometryLocation*> *insideLocs1 = ConnectedElementLocationFilter::getLocations(geom[1]);
		computeInside(insideLocs1, polys0, locPtPoly);
		if (minDistance <= 0.0) {
// flip locations, since we are testing geom 1 VS geom 0
			(*minDistanceLocation)[0] = (*locPtPoly)[1];
			(*minDistanceLocation)[1] = (*locPtPoly)[0];
			delete polys0;
			delete polys1;
			delete locPtPoly;
			for (unsigned int i=0; i<insideLocs1->size(); i++)
			{
				GeometryLocation *l = (*insideLocs1)[i];
				if ( l != (*minDistanceLocation)[0] &&
					l != (*minDistanceLocation)[1] )
				{
					delete l;
				}
			}
			delete insideLocs1;
			return;
		}
		for (unsigned int i=0; i<insideLocs1->size(); i++)
			delete (*insideLocs1)[i];
		delete insideLocs1;
	}
	delete polys0;
	delete polys1;
	delete locPtPoly;
}


void DistanceOp::computeInside(vector<GeometryLocation*> *locs,vector<Geometry*> *polys,vector<GeometryLocation*> *locPtPoly){
	for (unsigned int i=0;i<locs->size();i++) {
		GeometryLocation *loc=(*locs)[i];
		for (unsigned int j=0;j<polys->size();j++) {
			Polygon *poly=(Polygon*) (*polys)[j];
			computeInside(loc, poly, locPtPoly);
			if (minDistance<=0.0) return;
		}
	}
}

void DistanceOp::computeInside(GeometryLocation *ptLoc,Polygon *poly,vector<GeometryLocation*> *locPtPoly){
	Coordinate &pt=ptLoc->getCoordinate();
	if (Location::EXTERIOR!=ptLocator.locate(pt, poly)) {
		minDistance = 0.0;
		(*locPtPoly)[0] = ptLoc;
		GeometryLocation *locPoly = new GeometryLocation(poly, pt);
		(*locPtPoly)[1] = locPoly;
		return;
	}
}

void DistanceOp::computeLineDistance() {
	vector<GeometryLocation*> locGeom(2);
	/**
	* Geometries are not wholely inside, so compute distance from lines and points
	* of one to lines and points of the other
	*/
	vector<Geometry*> *lines0=LinearComponentExtracter::getLines(geom[0]);
	vector<Geometry*> *lines1=LinearComponentExtracter::getLines(geom[1]);
	vector<Geometry*> *pts0=PointExtracter::getPoints(geom[0]);
	vector<Geometry*> *pts1=PointExtracter::getPoints(geom[1]);

	// bail whenever minDistance goes to zero, since it can't get any less
	computeMinDistanceLines(lines0, lines1, &locGeom);
	updateMinDistance(&locGeom, false);
	if (minDistance <= 0.0) {
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistanceLinesPoints(lines0, pts1, &locGeom);
	updateMinDistance(&locGeom, false);
	if (minDistance <= 0.0) {
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistanceLinesPoints(lines1, pts0, &locGeom);
	updateMinDistance(&locGeom, true);
	if (minDistance <= 0.0){
		delete lines0;
		delete lines1;
		delete pts0;
		delete pts1;
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistancePoints(pts0, pts1, &locGeom);
	updateMinDistance(&locGeom, false);
	delete lines0;
	delete lines1;
	delete pts0;
	delete pts1;
}

void DistanceOp::computeMinDistanceLines(vector<Geometry*> *lines0,vector<Geometry*> *lines1,vector<GeometryLocation*> *locGeom){
	for (unsigned int i=0;i<lines0->size();i++) {
		LineString *line0=(LineString*) (*lines0)[i];
		for (unsigned int j=0;j<lines1->size();j++) {
			LineString *line1=(LineString*) (*lines1)[j];
			computeMinDistance(line0,line1,locGeom);
			if (minDistance<=0.0) return;
		}
	}
}

void
DistanceOp::computeMinDistancePoints(vector<Geometry*> *points0,vector<Geometry*> *points1,vector<GeometryLocation*> *locGeom)
{
	for (unsigned int i=0;i<points0->size();i++) {
		//Point *pt0=(Point*) (*points0)[i];
		Geometry *pt0=(*points0)[i];
		for (unsigned int j=0;j<points1->size();j++) {
			//Point *pt1=(Point*) (*points1)[j];
			Geometry *pt1=(*points1)[j];
			double dist=pt0->getCoordinate()->distance(*(pt1->getCoordinate()));
			if (dist < minDistance) {
				minDistance = dist;
				// this is wrong - need to determine closest points on both segments!!!
				(*locGeom)[0] = new GeometryLocation(pt0, 0, *(pt0->getCoordinate()));
				(*locGeom)[1] = new GeometryLocation(pt1, 0, *(pt1->getCoordinate()));
			}
			if (minDistance<=0.0) return;
			if ( i<points0->size()-1 || j<points1->size()-1)
			{
				delete (*locGeom)[0]; (*locGeom)[0]=NULL;
				delete (*locGeom)[1]; (*locGeom)[1]=NULL;
			}
		}
	}
}

void
DistanceOp::computeMinDistanceLinesPoints(vector<Geometry*> *lines,vector<Geometry*> *points,vector<GeometryLocation*> *locGeom)
{
	for (unsigned int i=0;i<lines->size();i++) {
		LineString *line=(LineString*) (*lines)[i];
		for (unsigned int j=0;j<points->size();j++) {
			Point *pt=(Point*)(*points)[j];
			computeMinDistance(line,pt,locGeom);
			if (minDistance<=0.0) return;
			if ( i<lines->size()-1 || j<points->size()-1)
			{
				delete (*locGeom)[0]; (*locGeom)[0]=NULL;
				delete (*locGeom)[1]; (*locGeom)[1]=NULL;
			}
		}
	}
}

void DistanceOp::computeMinDistance(const LineString *line0, const LineString *line1,vector<GeometryLocation*> *locGeom) {
	const Envelope *env0=line0->getEnvelopeInternal();
	const Envelope *env1=line1->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		//delete env0;
		//delete env1;
		return;
	}
	//delete env0;
	//delete env1;
	const CoordinateSequence *coord0=line0->getCoordinatesRO();
	const CoordinateSequence *coord1=line1->getCoordinatesRO();
	// brute force approach!
	for(int i=0;i<coord0->getSize()-1;i++) {
		for(int j=0;j<coord1->getSize()-1;j++) {
			double dist=CGAlgorithms::distanceLineLine(coord0->getAt(i),coord0->getAt(i+1),
				coord1->getAt(j),coord1->getAt(j+1));
			if (dist < minDistance) {
				minDistance = dist;
				LineSegment *seg0 = new LineSegment(coord0->getAt(i), coord0->getAt(i + 1));
				LineSegment *seg1 = new LineSegment(coord1->getAt(j), coord1->getAt(j + 1));
				CoordinateSequence* closestPt = seg0->closestPoints(seg1);
				delete seg0;
				delete seg1;
				Coordinate *c1 = new Coordinate(closestPt->getAt(0));
				Coordinate *c2 = new Coordinate(closestPt->getAt(1));
				newCoords.push_back(c1);
				newCoords.push_back(c2);
				delete closestPt;
				(*locGeom)[0] = new GeometryLocation(line0, i, *c1);
				(*locGeom)[1] = new GeometryLocation(line1, j, *c2);
			}
			if (minDistance<=0.0) return;
			if ( i<coord0->getSize()-1 || j<coord1->getSize()-1)
			{
				delete (*locGeom)[0]; (*locGeom)[0]=NULL;
				delete (*locGeom)[1]; (*locGeom)[1]=NULL;
			}
		}
	}
}

void
DistanceOp::computeMinDistance(const LineString *line, const Point *pt,vector<GeometryLocation*> *locGeom)
{
	const Envelope *env0=line->getEnvelopeInternal();
	const Envelope *env1=pt->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		//delete env0;
		//delete env1;
		return;
	}
	//delete env0;
	//delete env1;
	const CoordinateSequence *coord0=line->getCoordinatesRO();
	Coordinate *coord=new Coordinate(*(pt->getCoordinate()));
	newCoords.push_back(coord);
	// brute force approach!
	for(int i=0;i<coord0->getSize()-1;i++) {
		double dist=CGAlgorithms::distancePointLine(*coord,coord0->getAt(i),coord0->getAt(i+1));
        	if (dist < minDistance) {
          		minDistance = dist;
			LineSegment *seg = new LineSegment(coord0->getAt(i), coord0->getAt(i + 1));
			Coordinate *segClosestPoint = seg->closestPoint(*coord);
			delete seg;
			newCoords.push_back(segClosestPoint);
			delete (*locGeom)[0];
			(*locGeom)[0] = new GeometryLocation(line, i, *segClosestPoint);
			delete (*locGeom)[1];
			(*locGeom)[1] = new GeometryLocation(pt, 0, *coord);
        	}
		if (minDistance<=0.0) return;
	}
}

}

