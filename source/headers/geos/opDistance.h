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
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.10  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.9  2004/04/13 12:29:21  strk
 * GeometryLocation const-correctness.
 *
 * Revision 1.8  2004/04/13 10:05:51  strk
 * GeometryLocation constructor made const-correct.
 * Fixed erroneus down-casting in DistanceOp::computeMinDistancePoints.
 *
 * Revision 1.7  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_OPDISTANCE_H
#define GEOS_OPDISTANCE_H

#include <memory>
#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/geom.h>
#include <vector>

namespace geos {


/**
 * Represents the location of a point on a Geometry.
 * Maintains both the actual point location (which of course
 * may not be exact) as well as information about the component
 * and segment index where the point occurs.
 * Locations inside area Geometrys will not have an associated segment index,
 * so in this case the segment index will have the sentinel value of INSIDE_AREA.
 *
 */
class GeometryLocation {
private:
	const Geometry *component;
	int segIndex;
	Coordinate pt;
public:  
	/**
	* Special value of segment-index for locations inside area geometries. These
	* locations do not have an associated segment index.
	*/
	static const int INSIDE_AREA = -1;
	/**
	* Constructs a GeometryLocation specifying a point on a geometry, as well as the 
	* segment that the point is on (or INSIDE_AREA if the point is not on a segment).
	*/
	GeometryLocation(const Geometry *newComponent, int newSegIndex, const Coordinate &newPt);
	/**
	* Constructs a GeometryLocation specifying a point inside an area geometry.
	*/  
	GeometryLocation(const Geometry *newComponent, const Coordinate &newPt);
	/**
	* Returns the geometry associated with this location.
	*/
	const Geometry* getGeometryComponent();
	/**
	* Returns the segment index for this location. If the location is inside an
	* area, the index will have the value INSIDE_AREA;
	*
	* @return the segment index for the location, or INSIDE_AREA
	*/
	int getSegmentIndex();
	/**
	* Returns the location.
	*/
	Coordinate& getCoordinate();
	/**
	* Returns whether this GeometryLocation represents a point inside an area geometry.
	*/
	bool isInsideArea();
};


 /**
 * Extracts a single point
 * from each connected element in a Geometry
 * (e.g. a polygon, linestring or point)
 * and returns them in a list
 *
 */
class ConnectedElementPointFilter: public GeometryFilter {
public:
	/**
	* Returns a list containing a Coordinate from each Polygon, LineString, and Point
	* found inside the specified geometry. Thus, if the specified geometry is
	* not a GeometryCollection, an empty list will be returned.
	*/
	static vector<const Coordinate*>* getCoordinates(const Geometry *geom);
	ConnectedElementPointFilter(vector<const Coordinate*> *newPts);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom) {};
private:
	vector<const Coordinate*> *pts;
};

/**
 * A ConnectedElementPointFilter extracts a single point
 * from each connected element in a Geometry
 * (e.g. a polygon, linestring or point)
 * and returns them in a list. The elements of the list are 
 * {@link com.vividsolutions.jts.operation.distance.GeometryLocation}s.
 *
 */
class ConnectedElementLocationFilter: public GeometryFilter {
private:
	vector<GeometryLocation*> *locations;
public:
	/**
	* Returns a list containing a point from each Polygon, LineString, and Point
	* found inside the specified geometry. Thus, if the specified geometry is
	* not a GeometryCollection, an empty list will be returned. The elements of the list 
	* are {@link com.vividsolutions.jts.operation.distance.GeometryLocation}s.
	*/  
	static vector<GeometryLocation*>* getLocations(const Geometry *geom);
	ConnectedElementLocationFilter(vector<GeometryLocation*> *newLocations);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom);
};


/**
 * Computes the distance and
 * closest points between two {@link Geometry}s.
 * <p>
 * The distance computation finds a pair of points in the input geometries
 * which have minimum distance between them.  These points may
 * not be vertices of the geometries, but may lie in the interior of
 * a line segment. In this case the coordinate computed is a close
 * approximation to the exact point.
 * <p>
 * The algorithms used are straightforward O(n^2)
 * comparisons.  This worst-case performance could be improved on
 * by using Voronoi techniques.
 *
 */
class DistanceOp {
public:
	/**
	* Compute the distance between the closest points of two geometries.
	* @param g0 a {@link Geometry}
	* @param g1 another {@link Geometry}
	* @return the distance between the geometries
	*/
	static double distance(const Geometry *g0, const Geometry *g1);
	/**
	* Compute the the closest points of two geometries.
	* The points are presented in the same order as the input Geometries.
	*
	* @param g0 a {@link Geometry}
	* @param g1 another {@link Geometry}
	* @return the closest points in the geometries
	*/
	static CoordinateSequence* closestPoints(Geometry *g0,Geometry *g1);
	/**
	* Constructs a DistanceOp that computes the distance and closest points between
	* the two specified geometries.
	*/
	DistanceOp(const Geometry *g0, const Geometry *g1);
	~DistanceOp();
	/**
	* Report the distance between the closest points on the input geometries.
	*
	* @return the distance between the geometries
	*/
	double distance();
	/**
	* Report the coordinates of the closest points in the input geometries.
	* The points are presented in the same order as the input Geometries.
	*
	* @return a pair of {@link Coordinate}s of the closest points
	*/
	CoordinateSequence* closestPoints();
	/**
	* Report the locations of the closest points in the input geometries.
	* The locations are presented in the same order as the input Geometries.
	*
	* @return a pair of {@link GeometryLocation}s for the closest points
	*/
	vector<GeometryLocation*>* closestLocations();
private:
	PointLocator ptLocator;
	vector<Geometry const*> geom;
	vector<Coordinate *> newCoords;
	vector<GeometryLocation*> *minDistanceLocation;
	double minDistance;
	void updateMinDistance(double dist);
	void updateMinDistance(vector<GeometryLocation*> *locGeom, bool flip);
	void computeMinDistance();
	void DistanceOp::computeContainmentDistance();
	void computeInside(vector<GeometryLocation*> *locs,vector<Geometry*> *polys,vector<GeometryLocation*> *locPtPoly);
	void computeInside(GeometryLocation *ptLoc,Polygon *poly,vector<GeometryLocation*> *locPtPoly);
	void computeLineDistance();
	void computeMinDistanceLines(vector<Geometry*> *lines0,vector<Geometry*> *lines1,vector<GeometryLocation*> *locGeom);
	void computeMinDistancePoints(vector<Geometry*> *points0,vector<Geometry*> *points1,vector<GeometryLocation*> *locGeom);
	void computeMinDistanceLinesPoints(vector<Geometry*> *lines,vector<Geometry*> *points,vector<GeometryLocation*> *locGeom);
	void computeMinDistance(const LineString *line0, const LineString *line1,vector<GeometryLocation*> *locGeom);
	void computeMinDistance(const LineString *line, const Point *pt,vector<GeometryLocation*> *locGeom);
};
}
#endif
