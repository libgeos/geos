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
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_OPDISTANCE_H
#define GEOS_OPDISTANCE_H

#include <memory>
#include "platform.h"
#include "operation.h"
#include "geom.h"
#include <vector>

namespace geos {

/**
 * A PointExtracterFilter extracts all the component Points from a Geometry
 * and returns them in a list
 */
class PointExtracterFilter: public GeometryFilter {
public:
	static vector<const Geometry*>* getPoints(const Geometry *geom);
	PointExtracterFilter(vector<const Geometry*> *newPts);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom) {};
private:
	vector<const Geometry*> *pts;
};

/**
 * A LineExtracterFilter extracts all the component LineStrings from a Geometry
 * and returns them in a list
 */
class LineExtracterFilter: public GeometryComponentFilter {
public:
	static vector<const Geometry*>* getLines(const Geometry *geom);
	LineExtracterFilter(vector<const Geometry*> *newLines);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom) {};
private:
	vector<const Geometry*> *lines;
};

/**
 * A PolygonExtracterFilter extracts all the component Polygons from a Geometry
 * and returns them in a list
 */
class PolygonExtracterFilter: public GeometryFilter {
public:
	static vector<const Geometry*>* getPolygons(const Geometry *geom);
	PolygonExtracterFilter(vector<const Geometry*> *newComps);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom) {};
private:
	vector<const Geometry*> *comps;
};

/**
 * A ConnectedElementPointFilter extracts a single point
 * from each connected element in a Geometry
 * (e.g. a polygon, linestring or point)
 * and returns them in a list
 */
class ConnectedElementPointFilter: public GeometryFilter {
public:
	static vector<const Coordinate*>* getCoordinates(const Geometry *geom);
	ConnectedElementPointFilter(vector<const Coordinate*> *newPts);
	void filter_ro(const Geometry *geom);
	void filter_rw(Geometry *geom) {};
private:
	vector<const Coordinate*> *pts;
};

/**
 * DistanceOp computes the distance between two Geometries.
 * Currently the algorithms used are straightforward O(n^2)
 * comparisons.  These could definitely be improved on.
 */
class DistanceOp {
public:
	static double distance(const Geometry *g0, const Geometry *g1);
	DistanceOp(const Geometry *g0, const Geometry *g1);
	virtual ~DistanceOp();
	double distance();
private:
	PointLocator *ptLocator;
	vector<const Geometry*> *geom;
	double minDistance;
	void updateMinDistance(double dist);
	void computeMinDistance();
	void computeInside(vector<const Coordinate*> *pts,vector<const Geometry*> *polys);
	void computeInside(const Coordinate *pt,const Polygon *poly);
	void computeMinDistanceLines(vector<const Geometry*> *lines0,vector<const Geometry*> *lines1);
	void computeMinDistancePoints(vector<const Geometry*> *points0,vector<const Geometry*> *points1);
	void computeMinDistanceLinesPoints(vector<const Geometry*> *lines,vector<const Geometry*> *points);
	void computeMinDistance(const LineString *line0, const LineString *line1);
	void computeMinDistance(const LineString *line, const Point *pt);
};
}
#endif
