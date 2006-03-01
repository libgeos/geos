/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 **********************************************************************/

#include <geos/opDistance.h>
#include <geos/geomUtil.h>
#include <geos/geosAlgorithm.h>

using namespace geos::algorithm;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

using namespace geom;
using namespace geom::util;

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
	CoordinateSequence* closestPts=new CoordinateArraySequence();
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

void
DistanceOp::computeContainmentDistance()
{

	Polygon::ConstVect polys0;
	Polygon::ConstVect polys1;

	PolygonExtracter::getPolygons(*(geom[0]), polys0);
	PolygonExtracter::getPolygons(*(geom[1]), polys1);
	

	vector<GeometryLocation*> *locPtPoly = new vector<GeometryLocation*>(2);
	// test if either geometry is wholely inside the other
	if (polys1.size()>0) {
		vector<GeometryLocation*> *insideLocs0 = ConnectedElementLocationFilter::getLocations(geom[0]);
		computeInside(insideLocs0, polys1, locPtPoly);
		if (minDistance <= 0.0) {
			(*minDistanceLocation)[0] = (*locPtPoly)[0];
			(*minDistanceLocation)[1] = (*locPtPoly)[1];
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
	if (polys0.size()>0) {
		vector<GeometryLocation*> *insideLocs1 = ConnectedElementLocationFilter::getLocations(geom[1]);
		computeInside(insideLocs1, polys0, locPtPoly);
		if (minDistance <= 0.0) {
// flip locations, since we are testing geom 1 VS geom 0
			(*minDistanceLocation)[0] = (*locPtPoly)[1];
			(*minDistanceLocation)[1] = (*locPtPoly)[0];
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
	delete locPtPoly;
}


/*private*/
void
DistanceOp::computeInside(vector<GeometryLocation*> *locs,
		const Polygon::ConstVect& polys,
		vector<GeometryLocation*> *locPtPoly)
{
	for (unsigned int i=0, ni=locs->size(); i<ni; ++i)
	{
		GeometryLocation *loc=(*locs)[i];
		for (unsigned int j=0, nj=polys.size(); j<nj; ++j)
		{
			computeInside(loc, polys[j], locPtPoly);
			if (minDistance<=0.0) return;
		}
	}
}

/*private*/
void
DistanceOp::computeInside(GeometryLocation *ptLoc,
		const Polygon *poly,
		vector<GeometryLocation*> *locPtPoly)
{
	const Coordinate &pt=ptLoc->getCoordinate();
	if (Location::EXTERIOR!=ptLocator.locate(pt, static_cast<const Geometry *>(poly)))
	{
		minDistance = 0.0;
		(*locPtPoly)[0] = ptLoc;
		GeometryLocation *locPoly = new GeometryLocation(poly, pt);
		(*locPtPoly)[1] = locPoly;
		return;
	}
}

/*private*/
void
DistanceOp::computeLineDistance()
{
	vector<GeometryLocation*> locGeom(2);

	/**
	 * Geometries are not wholely inside, so compute distance from lines
	 * and points
	 * of one to lines and points of the other
	 */
	LineString::ConstVect lines0;
	LineString::ConstVect lines1;
	LinearComponentExtracter::getLines(*(geom[0]), lines0);
	LinearComponentExtracter::getLines(*(geom[1]), lines1);

	Point::ConstVect pts0;
	Point::ConstVect pts1;
	PointExtracter::getPoints(*(geom[0]), pts0);
	PointExtracter::getPoints(*(geom[1]), pts1);

	// bail whenever minDistance goes to zero, since it can't get any less
	computeMinDistanceLines(lines0, lines1, locGeom);
	updateMinDistance(&locGeom, false);
	if (minDistance <= 0.0) {
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistanceLinesPoints(lines0, pts1, locGeom);
	updateMinDistance(&locGeom, false);
	if (minDistance <= 0.0) {
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistanceLinesPoints(lines1, pts0, locGeom);
	updateMinDistance(&locGeom, true);
	if (minDistance <= 0.0){
		return;
	};

	locGeom[0]=NULL;
	locGeom[1]=NULL;
	computeMinDistancePoints(pts0, pts1, locGeom);
	updateMinDistance(&locGeom, false);
}

/*private*/
void
DistanceOp::computeMinDistanceLines(
		const LineString::ConstVect& lines0,
		const LineString::ConstVect& lines1,
		vector<GeometryLocation*>& locGeom)
{
	for (unsigned int i=0, ni=lines0.size(); i<ni; ++i)
	{
		const LineString *line0=lines0[i];
		for (unsigned int j=0, nj=lines1.size(); j<nj; ++j) {
			const LineString *line1=lines1[j];
			computeMinDistance(line0, line1, locGeom);
			if (minDistance<=0.0) return;
		}
	}
}

/*private*/
void
DistanceOp::computeMinDistancePoints(
		const Point::ConstVect& points0,
		const Point::ConstVect& points1,
		vector<GeometryLocation*>& locGeom)
{
	for (unsigned int i=0, ni=points0.size(); i<ni; ++i) {
		const Point *pt0=points0[i];
		//Geometry *pt0=(*points0)[i];
		for (unsigned int j=0, nj=points1.size(); j<nj; ++j) {
			const Point *pt1=points1[i];
			//Geometry *pt1=(*points1)[j];
			double dist=pt0->getCoordinate()->distance(*(pt1->getCoordinate()));
			if (dist < minDistance) {
				minDistance = dist;
				// this is wrong - need to determine closest points on both segments!!!
				locGeom[0] = new GeometryLocation(pt0, 0, *(pt0->getCoordinate()));
				locGeom[1] = new GeometryLocation(pt1, 0, *(pt1->getCoordinate()));
			}
			if (minDistance<=0.0) return;
			if ( i<points0.size()-1 || j<points1.size()-1)
			{
				delete locGeom[0]; locGeom[0]=NULL;
				delete locGeom[1]; locGeom[1]=NULL;
			}
		}
	}
}

/*private*/
void
DistanceOp::computeMinDistanceLinesPoints(
		const LineString::ConstVect& lines,
		const Point::ConstVect& points,
		vector<GeometryLocation*>& locGeom)
{
	for (unsigned int i=0;i<lines.size();i++) {
		const LineString *line=lines[i];
		for (unsigned int j=0;j<points.size();j++) {
			const Point *pt=points[j];
			computeMinDistance(line,pt,locGeom);
			if (minDistance<=0.0) return;
			if ( i<lines.size()-1 || j<points.size()-1)
			{
				delete locGeom[0]; locGeom[0]=NULL;
				delete locGeom[1]; locGeom[1]=NULL;
			}
		}
	}
}

/*private*/
void
DistanceOp::computeMinDistance(
		const LineString *line0,
		const LineString *line1,
		vector<GeometryLocation*>& locGeom)
{
	const Envelope *env0=line0->getEnvelopeInternal();
	const Envelope *env1=line1->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		return;
	}

	const CoordinateSequence *coord0=line0->getCoordinatesRO();
	const CoordinateSequence *coord1=line1->getCoordinatesRO();
	unsigned int npts0=coord0->getSize();
	unsigned int npts1=coord1->getSize();

	// brute force approach!
	for(unsigned int i=0; i<npts0-1; ++i)
	{
		for(unsigned int j=0; j<npts1-1; ++j)
		{
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
				locGeom[0] = new GeometryLocation(line0, i, *c1);
				locGeom[1] = new GeometryLocation(line1, j, *c2);
			}
			if (minDistance<=0.0) return;
			if ( i<npts0-1 || j<npts1-1)
			{
				delete locGeom[0]; locGeom[0]=NULL;
				delete locGeom[1]; locGeom[1]=NULL;
			}
		}
	}
}

/*private*/
void
DistanceOp::computeMinDistance(const LineString *line,
		const Point *pt,
		vector<GeometryLocation*>& locGeom)
{
	const Envelope *env0=line->getEnvelopeInternal();
	const Envelope *env1=pt->getEnvelopeInternal();
	if (env0->distance(env1)>minDistance) {
		return;
	}
	//delete env0;
	//delete env1;
	const CoordinateSequence *coord0=line->getCoordinatesRO();
	Coordinate *coord=new Coordinate(*(pt->getCoordinate()));
	newCoords.push_back(coord);
	// brute force approach!

	unsigned int npts0=coord0->getSize();
	for(unsigned int i=0; i<npts0-1; ++i)
	{
		double dist=CGAlgorithms::distancePointLine(*coord,coord0->getAt(i),coord0->getAt(i+1));
        	if (dist < minDistance) {
          		minDistance = dist;
			LineSegment *seg = new LineSegment(coord0->getAt(i), coord0->getAt(i + 1));
			Coordinate segClosestPoint;
			seg->closestPoint(*coord, segClosestPoint);
			delete seg;

			delete locGeom[0];
			locGeom[0] = new GeometryLocation(line, i, segClosestPoint);
			delete locGeom[1];
			locGeom[1] = new GeometryLocation(pt, 0, *coord);
        	}
		if (minDistance<=0.0) return;
	}
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.19  2006/03/01 17:16:39  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.18  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.17  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.16  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
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
 * Added CoordinateArraySequenceFactory::instance() function.
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

