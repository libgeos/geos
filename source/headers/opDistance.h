#ifndef GEOS_OPDISTANCE_H
#define GEOS_OPDISTANCE_H

#include "platform.h"
#include "operation.h"
#include "geom.h"
#include <vector>

/**
 * A PointExtracterFilter extracts all the component Points from a Geometry
 * and returns them in a list
 */
class PointExtracterFilter: public GeometryFilter {
public:
	static vector<Geometry*>* getPoints(Geometry *geom);
	PointExtracterFilter(vector<Geometry*> *newPts);
	void filter(Geometry *geom);
private:
	vector<Geometry*> *pts;
};

/**
 * A LineExtracterFilter extracts all the component LineStrings from a Geometry
 * and returns them in a list
 */
class LineExtracterFilter: public GeometryFilter {
public:
	static vector<Geometry*>* getLines(Geometry *geom);
	LineExtracterFilter(vector<Geometry*> *newLines);
	void filter(Geometry *geom);
private:
	vector<Geometry*> *lines;
};

/**
 * A PolygonExtracterFilter extracts all the component Polygons from a Geometry
 * and returns them in a list
 */
class PolygonExtracterFilter: public GeometryFilter {
public:
	static vector<Geometry*>* getPolygons(Geometry *geom);
	PolygonExtracterFilter(vector<Geometry*> *newComps);
	void filter(Geometry *geom);
private:
	vector<Geometry*> *comps;
};

/**
 * A ConnectedElementPointFilter extracts a single point
 * from each connected element in a Geometry
 * (e.g. a polygon, linestring or point)
 * and returns them in a list
 */
class ConnectedElementPointFilter: public GeometryFilter {
public:
	static vector<Coordinate*>* getCoordinates(Geometry *geom);
	ConnectedElementPointFilter(vector<Coordinate*> *newPts);
	void filter(Geometry *geom);
private:
	vector<Coordinate*> *pts;
};

/**
 * DistanceOp computes the distance between two Geometries.
 * Currently the algorithms used are straightforward O(n^2)
 * comparisons.  These could definitely be improved on.
 */
class DistanceOp {
public:
	static double distance(Geometry *g0,Geometry *g1);
	DistanceOp(Geometry *g0,Geometry *g1);
	virtual ~DistanceOp();
	double distance();
private:
	PointLocator *ptLocator;
	vector<Geometry*> *geom;
	double minDistance;
	void updateMinDistance(double dist);
	void computeMinDistance();
	void computeInside(vector<Coordinate*> *pts,vector<Geometry*> *polys);
	void computeInside(Coordinate *pt,Polygon *poly);
	void computeMinDistanceLines(vector<Geometry*> *lines0,vector<Geometry*> *lines1);
	void computeMinDistancePoints(vector<Geometry*> *points0,vector<Geometry*> *points1);
	void computeMinDistanceLinesPoints(vector<Geometry*> *lines,vector<Geometry*> *points);
	void computeMinDistance(LineString *line0,LineString *line1);
	void computeMinDistance(LineString *line,Point *pt);
};
#endif
