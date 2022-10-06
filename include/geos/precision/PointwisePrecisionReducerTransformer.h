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
class CoordinateSequence;
}
}

namespace geos {
namespace precision { // geos.precision


/**
 * A transformer to reduce the precision of a geometry pointwise.
 *
 * @author mdavis
 */
class GEOS_DLL PointwisePrecisionReducerTransformer : public geom::util::GeometryTransformer {

private:

    const geom::PrecisionModel& targetPM;

    std::unique_ptr<geom::CoordinateSequence> reducePointwise(
        const geom::CoordinateSequence* coordinates);

public:

    PointwisePrecisionReducerTransformer(
        const geom::PrecisionModel& p_targetPM)
        : targetPM(p_targetPM)
        {};

    static std::unique_ptr<geom::Geometry> reduce(
        const geom::Geometry& geom,
        const geom::PrecisionModel& targetPM);



protected:

    std::unique_ptr<geom::CoordinateSequence> transformCoordinates(
        const geom::CoordinateSequence* coords,
        const geom::Geometry* parent);


};

} // namespace geos.precision
} // namespace geos


