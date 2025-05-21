/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/edgegraph/MarkHalfEdge.h>
#include <geos/export.h>



// Forward declarations
namespace geos {
namespace geom {
    class CoordinateXYZM;
}
namespace edgegraph {
}
}


namespace geos {      // geos.
namespace dissolve {  // geos.dissolve


class GEOS_DLL DissolveHalfEdge : public edgegraph::MarkHalfEdge {


private:

    bool m_isStart = false;


public:

    DissolveHalfEdge(const geom::CoordinateXYZM& orig)
        : edgegraph::MarkHalfEdge(orig)
        , m_isStart(false)
    {}

    /**
     * Tests whether this edge is the starting segment
     * in a LineString being dissolved.
     *
     * @return true if this edge is a start segment
     */
    bool isStart();

    /**
     * Sets this edge to be the start segment of an input LineString.
     */
    void setStart();

};

} // namespace geos.dissolve
} // namespace geos

