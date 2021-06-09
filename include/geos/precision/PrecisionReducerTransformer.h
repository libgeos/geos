/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>

#include <geos/geom/util/GeometryTransformer.h>

// Forward declarations
namespace geos {
namespace geom {
class PrecisionModel;
class Geometry;
}
}

namespace geos {
namespace precision { // geos.precision

/** \brief
 * Reduces the precision of a {@link geom::Geometry}
 * according to the supplied {@link geom::PrecisionModel}, without
 * attempting to preserve valid topology.
 *
 * The topology of the resulting geometry may be invalid if
 * topological collapse occurs due to coordinates being shifted.
 * It is up to the client to check this and handle it if necessary.
 * Collapses may not matter for some uses.  An example
 * is simplifying the input to the buffer algorithm.
 * The buffer algorithm does not depend on the validity of the input geometry.
 *
 */
class GEOS_DLL PrecisionReducerTransformer : public geom::util::GeometryTransformer {

private:

    const geom::PrecisionModel& targetPM;
    bool isRemoveCollapsed;

    std::unique_ptr<geom::Geometry> reduceArea(const geom::Geometry* geom);

    void extend(
        std::vector<geom::Coordinate>& coords,
        std::size_t minLength);


public:

    PrecisionReducerTransformer(
        const geom::PrecisionModel& p_targetPM,
        bool p_isRemoveCollapsed = false)
        : targetPM(p_targetPM)
        , isRemoveCollapsed(p_isRemoveCollapsed)
        {};

    static std::unique_ptr<geom::Geometry> reduce(
        const geom::Geometry& geom,
        const geom::PrecisionModel& targetPM,
        bool isRemoveCollapsed = false);


protected:

    std::unique_ptr<geom::CoordinateSequence> transformCoordinates(
        const geom::CoordinateSequence* coords,
        const geom::Geometry* parent) override;

    std::unique_ptr<geom::Geometry> transformPolygon(
        const geom::Polygon* geom,
        const geom::Geometry* parent) override;

    std::unique_ptr<geom::Geometry> transformMultiPolygon(
        const geom::MultiPolygon* geom,
        const geom::Geometry* parent) override;


};

} // namespace geos.precision
} // namespace geos


