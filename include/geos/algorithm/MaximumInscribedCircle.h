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
        const geom::Polygon* poly;
        unsigned int inputNumSegments;
        std::vector<const geom::Point*> voronoiVertices;
        geom::Coordinate centre;
        double radius;

        void addRingSites(std::vector<geom::Coordinate>* sites, const geom::LineString* ring);
        void compute();
        void computeVoronoiVertices();
        void computeCentreAndRadius();
        std::vector<geom::Coordinate>* computeSites();
        const geom::Polygon* polygonOfMaxArea(const geom::MultiPolygon* multiPoly);

    public:

        MaximumInscribedCircle(const geom::Geometry* geom, unsigned int numSegments):
            input(nullptr),
            inputNumSegments(2),
            radius(0.0)
        {
            input = geom;
            inputNumSegments = numSegments;
            centre.setNull();
        }

        ~MaximumInscribedCircle() {}; // This is a destructor and is required to free resources

        /**
         * Gets a geometry which represents the Maximum Inscribed Circle.
         * If the input is degenerate (empty), this method will return an empty
         * geometry. If the input is a single Point, then the input Point will
         * be returned. If the input is a Polygon, it will return the approximation
         * of the Maximum Inscribed Circle for the given polygon. If the input is a
         * MultiPolygon, then it will return the approximation of the Maximum
         * Inscribed Circle for the polygon with the largest area. In all other
         * cases an empty geometry will be returned.
         *
         * (Note that because this algorithm only approximates the medial axis
         * via Voronoi, the result will not necessarily be the true Maximum
         * Inscribed Circle.)
         *
         * @return A Geometry representing the Maximum Inscribed Circle.
         */
        std::unique_ptr<geom::Geometry> getCircle();

        /**
         * Gets the centre point of the computed Maximum Inscribed Circle.
         *
         * @return the centre point of the Maximum Incribed Circle
         * @return null if the input is degenerate or not a Point, Polygon or MultiPolygon
         */
        geom::Coordinate getCentre();

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
