/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/MaximumInscribedCircle.java
 * https://github.com/locationtech/jts/commit/98274a7ea9b40651e9de6323dc10fb2cac17a245
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_CONSTRUCT_MAXIMUMCIRCLE_H
#define GEOS_ALGORITHM_CONSTRUCT_MAXIMUMCIRCLE_H

#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/Envelope.h>

#include <memory>
#include <queue>



namespace geos {
namespace geom {
class Coordinate;
class Envelope;
class Geometry;
class GeometryFactory;
class LineString;
class Point;
}
namespace operation {
namespace distance {
class IndexedFacetDistance;
}
}
namespace algorithm {
namespace locate {
class IndexedPointInAreaLocator;
}
}
}


namespace geos {
namespace algorithm { // geos::algorithm
namespace construct { // geos::algorithm::construct

/**
 * Computes the Euclidean distance (L2 metric) from a Point to a Geometry.
 *
 * Also computes two points which are separated by the distance.
 */
class MaximumInscribedCircle {

public:

    MaximumInscribedCircle(const geom::Geometry *polygonal, double tolerance);

    /**
    * Gets the center point of the maximum inscribed circle
    * (up to the tolerance distance).
    *
    * @return the center point of the maximum inscribed circle
    */
    std::unique_ptr<geom::Point> getCenter();

    /**
    * Gets a point defining the radius of the Maximum Inscribed Circle.
    * This is a point on the boundary which is
    * nearest to the computed center of the Maximum Inscribed Circle.
    * The line segment from the center to this point
    * is a radius of the constructed circle, and this point
    * lies on the boundary of the circle.
    *
    * @return a point defining the radius of the Maximum Inscribed Circle
    */
    std::unique_ptr<geom::Point> getRadiusPoint();

    /**
    * Gets a line representing a radius of the Largest Empty Circle.
    *
    * @return a line from the center of the circle to a point on the edge
    */
    std::unique_ptr<geom::LineString> getRadiusLine();

    /**
    * Computes the center point of the Maximum Inscribed Circle
    * of a polygonal geometry, up to a given tolerance distance.
    *
    * @param polygonal a polygonal geometry
    * @param tolerance the distance tolerance for computing the center point
    * @return the center point of the maximum inscribed circle
    */
    static std::unique_ptr<geom::Point> getCenter(const geom::Geometry *polygonal, double tolerance);

    /**
    * Computes a radius line of the Maximum Inscribed Circle
    * of a polygonal geometry, up to a given tolerance distance.
    *
    * @param polygonal a polygonal geometry
    * @param tolerance the distance tolerance for computing the center point
    * @return a line from the center to a point on the circle
    */
    static std::unique_ptr<geom::LineString> getRadiusLine(const geom::Geometry *polygonal, double tolerance);

private:

    /* private class */
    class Cell {
        private:
            static constexpr double SQRT2 = 1.4142135623730951;
            double x;
            double y;
            double hSide;
            double distance;
            double maxDist;

        public:
            Cell(double p_x, double p_y, double p_hSide, double p_distanceToBoundary)
                    : x(p_x)
                    , y(p_y)
                    , hSide(p_hSide)
                    , distance(p_distanceToBoundary)
                    , maxDist(p_distanceToBoundary*p_hSide*SQRT2)
                    {};

            geom::Envelope getEnvelope() {
                geom::Envelope env(x-hSide, x+hSide, y-hSide, y+hSide);
                return env;
            }

            double getMaxDistance() {return maxDist; }
            double getDistance() { return distance; }
            double getHSide() { return hSide; }
            double getX() { return x; }
            double getY() { return y; }
            bool operator<(const Cell &rhs) const { return maxDist < rhs.maxDist; }
            bool operator>(const Cell &rhs) const { return maxDist > rhs.maxDist; }
            bool operator==(const Cell &rhs) const { return maxDist == rhs.maxDist; }
    };

    /* private members */
    const geom::Geometry *inputGeom;
    double tolerance;
    const geom::GeometryFactory *factory;
    std::unique_ptr<algorithm::locate::IndexedPointInAreaLocator> ptLocater;
    std::unique_ptr<operation::distance::IndexedFacetDistance> indexedDistance;
    geom::Coordinate centerPt;
    geom::Coordinate radiusPt;
    bool done;

    /* private methods */
    double distanceToBoundary(const geom::Coordinate &c);
    double distanceToBoundary(double x, double y);
    void compute();
    void createInitialGrid(const geom::Envelope *env, std::priority_queue<Cell> &cellQueue);
    Cell createCell(double x, double y, double hSide);
    Cell createCentroidCell(const geom::Geometry *geom);

};


} // geos::algorithm::construct
} // geos::algorithm
} // geos

#endif // GEOS_ALGORITHM_CONSTRUCT_MAXIMUMCIRCLE_H
