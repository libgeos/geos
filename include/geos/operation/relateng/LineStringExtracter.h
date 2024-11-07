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

#include <geos/geom/GeometryFilter.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
    class LineString;
    class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


class GEOS_DLL LineStringExtracter : public geos::geom::GeometryFilter {
    using LineString = geos::geom::LineString;
    using Geometry = geos::geom::Geometry;

private:

    std::vector<const LineString*>& comps;


public:

    LineStringExtracter(std::vector<const LineString*>& p_comps)
        : comps(p_comps)
        {}

    void filter_ro(const geom::Geometry* geom) override;

    static void getLines(const Geometry* geom, std::vector<const LineString*>& lines);

    static std::vector<const LineString*> getLines(const Geometry* geom);

    /**
    * Extracts the {@link LineString} elements from a single {@link Geometry}
    * and returns them as either a {@link LineString} or {@link MultiLineString}.
    *
    * @param geom the geometry from which to extract
    * @return a linear geometry
    */
    // static std::unique_ptr<Geometry> getGeometry(const Geometry* geom);

};


} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

