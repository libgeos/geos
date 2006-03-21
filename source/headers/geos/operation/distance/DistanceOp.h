/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OP_DISTANCE_DISTANCEOP_H
#define GEOS_OP_DISTANCE_DISTANCEOP_H

#include <geos/algorithm/PointLocator.h> // for composition

#include <vector>

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
		class Polygon;
		class LineString;
		class Point;
		class Geometry;
		class CoordinateSequence;
	}
	namespace operation { 
		namespace distance { 
			class GeometryLocation;
		}
	}
}


namespace geos {
namespace operation { // geos::operation
namespace distance { // geos::operation::distance

/** \brief
 * Computes the distance and
 * closest points between two {@link Geometry}s.
 * 
 * The distance computation finds a pair of points in the input geometries
 * which have minimum distance between them.  These points may
 * not be vertices of the geometries, but may lie in the interior of
 * a line segment. In this case the coordinate computed is a close
 * approximation to the exact point.
 *
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
	static double distance(const geom::Geometry *g0, const geom::Geometry *g1);

	/**
	 * Compute the the closest points of two geometries.
	 * The points are presented in the same order as the input Geometries.
	 *
	 * @param g0 a {@link Geometry}
	 * @param g1 another {@link Geometry}
	 * @return the closest points in the geometries
	 */
	static geom::CoordinateSequence* closestPoints(geom::Geometry *g0, geom::Geometry *g1);

	/**
	 * Constructs a DistanceOp that computes the distance and closest points between
	 * the two specified geometries.
	 */
	DistanceOp(const geom::Geometry *g0, const geom::Geometry *g1);

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
	geom::CoordinateSequence* closestPoints();

	/**
	 * Report the locations of the closest points in the input geometries.
	 * The locations are presented in the same order as the input Geometries.
	 *
	 * @return a pair of {@link GeometryLocation}s for the closest points
	 */
	std::vector<GeometryLocation*>* closestLocations();

private:

	algorithm::PointLocator ptLocator;
	std::vector<geom::Geometry const*> geom;
	std::vector<geom::Coordinate *> newCoords;
	std::vector<GeometryLocation*> *minDistanceLocation;
	double minDistance;
	void updateMinDistance(double dist);
	void updateMinDistance(std::vector<GeometryLocation*> *locGeom, bool flip);
	void computeMinDistance();
	void computeContainmentDistance();

	void computeInside(std::vector<GeometryLocation*> *locs,
			const std::vector<const geom::Polygon*>& polys,
			std::vector<GeometryLocation*> *locPtPoly);

	void computeInside(GeometryLocation *ptLoc,
			const geom::Polygon *poly,
			std::vector<GeometryLocation*> *locPtPoly);

	void computeLineDistance();

	void computeMinDistanceLines(
			const std::vector<const geom::LineString*>& lines0,
			const std::vector<const geom::LineString*>& lines1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistancePoints(
			const std::vector<const geom::Point*>& points0,
			const std::vector<const geom::Point*>& points1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistanceLinesPoints(
			const std::vector<const geom::LineString*>& lines0,
			const std::vector<const geom::Point*>& points1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistance(const geom::LineString *line0,
			const geom::LineString *line1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistance(const geom::LineString *line,
			const geom::Point *pt,
			std::vector<GeometryLocation*>& locGeom);
};


} // namespace geos::operation::distance
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_DISTANCE_DISTANCEOP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 **********************************************************************/

