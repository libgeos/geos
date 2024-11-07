/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/IndexedDistanceToPoint.java
 * https://github.com/locationtech/jts/commit/d92f783163d9440fcc10c729143787bf7b9fe8f9
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/algorithm/construct/IndexedPointInPolygonsLocator.h>
#include <geos/operation/distance/IndexedFacetDistance.h>

namespace geos {
namespace algorithm { // geos::algorithm
namespace construct { // geos::algorithm::construct

/**
 * \brief Computes the distance between a point and a geometry
 * (which may be a collection containing any type of geometry).
 * 
 * Also computes the pair of points containing the input
 * point and the nearest point on the geometry.
 * 
 * \author Martin Davis
 */
class GEOS_DLL IndexedDistanceToPoint {
    using Geometry = geos::geom::Geometry;
    using Point = geos::geom::Point;
    using IndexedFacetDistance = geos::operation::distance::IndexedFacetDistance;

public:
    /** 
     * \brief Creates an instance to find the distance from points to a geometry.
     * 
     * \param geom the geometry to compute distances to
     */
    IndexedDistanceToPoint(const Geometry& geom);

    /**
     * \brief Computes the distance from the base geometry to the given point.
     *
     * \param pt the point to compute the distance to
     *
     * \return the computed distance
     */
    double distance(const Point& pt);

    /**
     * \brief Computes the nearest point on the geometry to the point.
     * 
     * The first location lies on the geometry, 
     * and the second location is the provided point.
     *
     * \param pt the point to compute the nearest point for
     *
     * \return the points that are nearest
     */
    std::unique_ptr<geom::CoordinateSequence> nearestPoints(const Point& pt);

private:
    void init();

    bool isInArea(const Point& pt);

    //-- members
    const Geometry& targetGeometry;
    std::unique_ptr<operation::distance::IndexedFacetDistance> facetDistance;
    std::unique_ptr<IndexedPointInPolygonsLocator> ptLocator;

};

}}}