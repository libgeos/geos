/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Jonathan Adams <jd.adams16@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_MAXIMUMINSCRIBEDCIRCLE_H
#define GEOS_ALGORITHM_MAXIMUMINSCRIBEDCIRCLE_H

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>

namespace geos {
namespace algorithm { // geos::algorithm

class GEOS_DLL MaximumInscribedCircle {

    private:

        // member variables
        const geom::Geometry* input;
        int inputNumSegments;
        std::vector<geom::Coordinate> sites;
        std::vector<geom::Coordinate> voronoiVertices;
        geom::Coordinate center;
        double radius;
        double xmin, xmax, ymin, ymax;

        void addRingSites(const geom::LineString* ring);
        void compute();
        void computeSites(const geom::Polygon* poly);
        void computeVoronoiVertices();
        void computeCenterAndRadius(geom::Polygon& poly);
        const geom::Polygon* polygonOfMaxArea(const geom::MultiPolygon* multiPoly);

    public:

        MaximumInscribedCircle(const geom::Geometry* geom, const int numSegments):
            input(nullptr),
            inputNumSegments(2),
            radius(0.0)
        {
            input = geom;
            inputNumSegments = numSegments;
            center.setNull();
        }

        ~MaximumInscribedCircle() {}; // This is a destructor and is required to free resources

        /**
         * Gets a geometry which represents the Maximum Inscribed Circle.
         * If the input is degenerate (empty or a single unique point),
         * this method will return an empty geometry or a single Point geometry.
         * If the input is a Polygon, it will return the approximation of the
         * Maximum Inscribed Circle for the given polygon. If the input is a
         * MultiPolygon, then it will return the approximation of the Maximum
         * Inscribed Circle for the polygon with the largest area. In all other
         * cases an empty geometry will be returned.
         *
         * If the input is a Geometry Collection, then each geometry will be
         * run through the algorithm and a Geometry Collection of all inscribed
         * circles will be returned. Otherwise, a Polygon will be returned which
         * approximates the Maximum Inscribed Circle.
         *
         * (Note that because this algorithm only approximates the medial axis
         * via Voronoi, the result will not necessarily be the true Maximum
         * Inscribed Circle.)
         *
         * @return A Geometry representing the Maximum Inscribed Circle.
         */
        std::unique_ptr<geom::Geometry> getCircle();

        /**
         * Gets the center point of the computed Maximum Inscribed Circle.
         *
         * @return the center point of the Maximum Incribed Circle
         * @return null if the input is empty
         */
        geom::Coordinate getCenter();

        /**
         * Gets the radius of the computed Maximum Inscribed Circle.
         *
         * @return the radius of the Maximum Inscribed Circle
         */
        double getRadius();

};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_MAXIMUMINSCRIBEDCIRCLE_H
