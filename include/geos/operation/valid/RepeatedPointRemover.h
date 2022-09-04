/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>


namespace geos {
namespace operation {
namespace valid {

    /**
     *  Removes repeated, consecutive equal, coordinates from a CoordinateSequence.
     */
    class GEOS_DLL RepeatedPointRemover {

    public:

        /**
         *  Returns a new CoordinateSequence being a copy of the input
         *  with any consecutive equal Coordinate removed.
         *  Equality test is 2D based.
         *
         *  \param seq to filter
         *  \param tolerance to apply
         *  \return Geometr, ownership of returned object goes to the caller.
         */
        static std::unique_ptr<geom::CoordinateSequence>
            removeRepeatedPoints(
                const geom::CoordinateSequence* seq,
                double tolerance = 0.0);

        static std::unique_ptr<geom::CoordinateSequence>
            removeRepeatedAndInvalidPoints(
                const geom::CoordinateSequence* seq,
                double tolerance = 0.0);

        static std::unique_ptr<geom::Geometry>
            removeRepeatedPoints(
                const geom::Geometry* geom,
                double tolerance = 0.0);
    };

}
}
}

