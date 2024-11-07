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

#include <geos/geom/Coordinate.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace algorithm {
    class BoundaryNodeRule;
}
namespace geom {
    class CoordinateXY;
    class LineString;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

class GEOS_DLL LinearBoundary {
    using BoundaryNodeRule = geos::algorithm::BoundaryNodeRule;
    using Coordinate = geos::geom::Coordinate;
    using CoordinateXY = geos::geom::CoordinateXY;
    using LineString = geos::geom::LineString;

private:

    // Members

    Coordinate::ConstIntMap m_vertexDegree;
    bool m_hasBoundary;
    const BoundaryNodeRule& m_boundaryNodeRule;


public:

    // Constructors

    LinearBoundary(std::vector<const LineString*>& lines, const BoundaryNodeRule& bnRule);

    bool hasBoundary() const;

    bool isBoundary(const CoordinateXY* pt) const;


private:

    // Methods

    bool checkBoundary(Coordinate::ConstIntMap& vertexDegree) const;

    static void computeBoundaryPoints(
        std::vector<const LineString*>& lines,
        Coordinate::ConstIntMap& vertexDegree);

    static void addEndpoint(
        const CoordinateXY *p,
        Coordinate::ConstIntMap& vertexDegree);


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

