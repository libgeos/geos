/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <vector>

namespace geos::operation::grid {

class GEOS_DLL TraversalAreas {
public:
    /**
     * @brief Return the total area of counter-clockwise closed rings formed by this box and the provided Coordinate sequences
     *
     * @param box boundary of the area to consider (Cell)
     * @param coord_lists vector of Coordinate vectors representing points that traverse `box`. Either the first and
     *                    last coordinate of each vector must lie on the boundary of `box`, or the coordinates
     *                    must form a closed ring that does not intersect the boundary of `box.` Clockwise-oriented
     *                    closed rings will be considered holes.
     * @return total area
     */
    static double
    getLeftHandArea(const geom::Envelope& box, const std::vector<const std::vector<geom::CoordinateXY>*>& coord_lists);

    /**
     * @brief Return an areal geometry representing the closed rings formed by this box and the provided Coordinate sequences
     *
     * @param gfact GeometryFactory to create result geometry
     * @param box boundary of the area to consider (Cell)
     * @param coord_lists vector of Coordinate vectors representing points that traverse `box`. Either the first and
     *                    last coordinate of each vector must lie on the boundary of `box`, or the coordinates
     *                    must form a closed ring that does not intersect the boundary of `box.` Clockwise-oriented
     *                    closed rings will be considered holes.
     * @return a Polygon or MultiPolygon geometry
     */
    static std::unique_ptr<geom::Geometry>
    getLeftHandRings(const geom::GeometryFactory& gfact, const geom::Envelope& box, const std::vector<const std::vector<geom::CoordinateXY>*>& coord_lists);

};

}
