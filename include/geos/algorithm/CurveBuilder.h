/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025-2026 ISciences, LLC
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

namespace geos::geom {
    class CircularArc;
    class CoordinateSequence;
    class Curve;
    class GeometryFactory;
    class LineString;
    class SimpleCurve;
}

namespace geos::algorithm {

class GEOS_DLL CurveBuilder {

public:

    static std::unique_ptr<geom::Curve> getCurved(const geom::LineString& ls, double distanceTolerance);

private:

    explicit CurveBuilder(const geom::GeometryFactory& factory);

    std::unique_ptr<geom::Curve> compute(const geom::LineString& ls, double distanceTolerance);

    void addArc(const geom::CircularArc& arc, std::size_t stop);

    void addLineCoords(const geom::CoordinateSequence& points, std::size_t from, std::size_t to);

    void finishArc();

    void finishLine();

    std::shared_ptr<geom::CoordinateSequence> lineCoords;
    std::shared_ptr<geom::CoordinateSequence> arcCoords;
    std::vector<std::unique_ptr<geom::SimpleCurve>> curves;
    const geom::GeometryFactory& factory;

    /// Maximum angle between two vertices to be considered part of the same arc.
    static constexpr double maxSpacingRadians = MATH_PI / 4 + 0.01;

    /// Declared as non-copyable
    CurveBuilder(const CurveBuilder& other);
    CurveBuilder& operator=(const CurveBuilder& rhs);
};

}