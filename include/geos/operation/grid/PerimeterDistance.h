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

namespace geos::geom {
    class CoordinateXY;
    class Envelope;
}

namespace geos::operation::grid {

class GEOS_DLL PerimeterDistance {
    public:

    /** Calculates the clockwise distance along the edge of an Envelope
     *  from the lower left corner of the Envelope to a supplied Coordinate
     *  lying exactly on the boundary of the Envelope.
     */
    static double
    getPerimeterDistance(const geom::Envelope& env, const geom::CoordinateXY& c);

    static double
    getPerimeterDistance(double xmin, double ymin, double xmax, double ymax, double x, double y);

    /** Calculates the counter-clockwise distance between two locations on the perimeter
     *  of an Envelope. The locations are specified as clockwise distances from the
     *  lower left corner of the Envelope, consistent with the `perimeter_distance`
     *  function.
     */
    static double
    getPerimeterDistanceCCW(double measure1, double measure2, double perimeter);

};

}
