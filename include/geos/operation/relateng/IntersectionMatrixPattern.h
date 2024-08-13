/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <string>


namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


class GEOS_DLL IntersectionMatrixPattern {

private:

    /**
     * Cannot be instantiated.
     */
    IntersectionMatrixPattern() {};


public:

    /**
     * A DE-9IM pattern to detect whether two polygonal geometries are adjacent along
     * an edge, but do not overlap.
     */
    static constexpr const char* ADJACENT = "F***1****";

    /**
     * A DE-9IM pattern to detect a geometry which properly contains another
     * geometry (i.e. which lies entirely in the interior of the first geometry).
     */
    static constexpr const char* CONTAINS_PROPERLY = "T**FF*FF*";

    /**
     * A DE-9IM pattern to detect if two geometries intersect in their interiors.
     * This can be used to determine if a polygonal coverage contains any overlaps
     * (although not whether they are correctly noded).
     */
    static constexpr const char* INTERIOR_INTERSECTS = "T********";


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

