/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
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
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

using geos::geom::Geometry;

namespace geos {      // geos
namespace coverage { // geos::coverage

/**
 * Unions a polygonal coverage in an efficient way.
 *
 * Valid polygonal coverage topology allows merging polygons in a very efficient way.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL CoverageUnion {

private:


public:

    /**
    * Unions a polygonal coverage.
    *
    * @param coverage a vector of polygons in the coverage
    * @return the union of the coverage polygons
    */
    static std::unique_ptr<Geometry> Union(std::vector<const Geometry*>& coverage);

    /**
    * Unions a polygonal coverage.
    *
    * @param coverage a collection of the polygons in the coverage
    * @return the union of the coverage polygons
    */
    static std::unique_ptr<Geometry> Union(const Geometry* coverage);

};

} // namespace geos::coverage
} // namespace geos










