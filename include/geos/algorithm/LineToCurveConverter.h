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
#include <geos/geom/util/CurveBuilder.h>

#include <memory>

namespace geos::algorithm {
    class LineToCurveParams;
}

namespace geos::geom {
    class CircularArc;
    class CoordinateSequence;
    class Curve;
    class GeometryFactory;
    class LineString;
    class SimpleCurve;
}

namespace geos::algorithm {

class GEOS_DLL LineToCurveConverter {

public:

    /** Construct a curved geometry from the provided LineString. The provided LineToCurveParams will be used
     *  to determine when successive vertices of the LineString can be interpreted as a circular arc.
     */
    static std::unique_ptr<geom::Curve> getCurved(const geom::LineString& ls, const LineToCurveParams& params);

private:

    explicit LineToCurveConverter(const geom::GeometryFactory& factory, bool outputZ, bool outputM);

    std::unique_ptr<geom::Curve> compute(const geom::LineString& ls, const LineToCurveParams& params);

    void addArc(const geom::CircularArc& arc, std::size_t stop);

    void addLineCoords(const geom::CoordinateSequence& points, std::size_t from, std::size_t to);

    /*

    void finishArc();

    void finishLine();

    std::shared_ptr<geom::CoordinateSequence> lineCoords;
    std::shared_ptr<geom::CoordinateSequence> arcCoords;
    std::vector<std::unique_ptr<geom::SimpleCurve>> curves;
    const geom::GeometryFactory& factory;
    */
    geom::util::CurveBuilder curveBuilder;

    /// Declared as non-copyable
    LineToCurveConverter(const LineToCurveConverter& other);
    LineToCurveConverter& operator=(const LineToCurveConverter& rhs);
};

}