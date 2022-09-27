/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#pragma once

#include <geos/geom/prep/PreparedPolygonPredicate.h>

// Forward declarations
namespace geos {
    namespace geom {
        class Geometry;
        namespace prep {
            class PreparedPolygon;
        }
    }
}

namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

class PreparedPolygon;

class PreparedPolygonDistance : public PreparedPolygonPredicate {
public:

    static double distance(const PreparedPolygon& prep, const geom::Geometry* geom)
    {
        PreparedPolygonDistance op(prep);
        return op.distance(geom);
    }

    PreparedPolygonDistance(const PreparedPolygon& prep)
        : PreparedPolygonPredicate(&prep)
    { }

    double distance(const geom::Geometry* g) const;

    bool isWithinDistance(const geom::Geometry* g, double d) const;

protected:

    // Declare type as noncopyable
    PreparedPolygonDistance(const PreparedPolygonDistance& other) = delete;
    PreparedPolygonDistance& operator=(const PreparedPolygonDistance& rhs) = delete;
};

} // namespace geos::geom::prep
} // namespace geos::geom
} // namespace geos

